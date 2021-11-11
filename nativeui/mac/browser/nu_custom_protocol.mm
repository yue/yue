// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/browser/nu_custom_protocol.h"

#include <map>

#include "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"
#include "base/synchronization/lock.h"

namespace {

// Whether we have registered this protocol class.
bool g_initailized = false;

// A map of schemes and handlers.
std::map<std::string, nu::Browser::ProtocolHandler> g_handlers;

// Lock to guard the handlers map.
base::Lock g_lock;

}  // namespace

@implementation NUCustomProtocol

+ (bool)registerProtocol:(NSString*)scheme
             withHandler:(nu::Browser::ProtocolHandler)handler {
  if (!g_initailized) {
    [NSURLProtocol registerClass:[NUCustomProtocol class]];
    g_initailized = true;
  }
  {
    base::AutoLock auto_lock(g_lock);
    g_handlers[[scheme UTF8String]] = std::move(handler);
  }
  // This private API can make WKWebview aware of our custom protocol class.
  Class cls = NSClassFromString(@"WKBrowsingContextController");
  SEL sel = NSSelectorFromString(@"registerSchemeForCustomProtocol:");
  if (![cls respondsToSelector:sel])
    return false;
  [cls performSelector:sel withObject:scheme];
  return true;
}

+ (bool)unregisterProtocol:(NSString*)scheme {
  {
    base::AutoLock auto_lock(g_lock);
    g_handlers.erase([scheme UTF8String]);
  }
  // The private API to unregister protocol.
  Class cls = NSClassFromString(@"WKBrowsingContextController");
  SEL sel = NSSelectorFromString(@"unregisterSchemeForCustomProtocol:");
  if (![cls respondsToSelector:sel])
    return false;
  [cls performSelector:sel withObject:scheme];
  return true;
}

#pragma mark - NSURLProtocol

+ (BOOL)canInitWithRequest:(NSURLRequest*)request {
  base::AutoLock auto_lock(g_lock);
  return g_handlers.find([request.URL.scheme UTF8String]) != g_handlers.end();
}

+ (NSURLRequest*)canonicalRequestForRequest:(NSURLRequest*)request {
  return request;
}

- (void)dealloc {
  if (protocol_job_) {
    // Release the job in main thread.
    __block nu::ProtocolJob* job = protocol_job_;
    dispatch_async(dispatch_get_main_queue(), ^{
      job->Release();
    });
  }
  [super dealloc];
}

- (void)startLoading {
  // Create job in main thread.
  __block nu::ProtocolJob* job = nullptr;
  __block std::string scheme([self.request.URL.scheme UTF8String]);
  __block std::string url([[self.request.URL absoluteString] UTF8String]);
  dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
  dispatch_async(dispatch_get_main_queue(), ^{
    nu::Browser::ProtocolHandler handler;
    {
      base::AutoLock auto_lock(g_lock);
      auto it = g_handlers.find(scheme);
      if (it != g_handlers.end())
        handler = it->second;
    }
    if (handler) {
      job = handler(url);
      job->AddRef();
    }
    // Wake up the thread that waits for the job.
    dispatch_semaphore_signal(semaphore);
  });

  // Wait for the protocol job.
  dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
  protocol_job_ = job;
  if (!protocol_job_) {
    NSError* error = [NSError errorWithDomain:NSURLErrorDomain
                                         code:NSURLErrorUnsupportedURL
                                     userInfo:nil];
    [[self client] URLProtocol:self didFailWithError:error];
    return;
  }

  // Start.
  protocol_job_->Plug([&](int size) {
    std::string mime_type;
    protocol_job_->GetMimeType(&mime_type);
    // Send response.
    base::scoped_nsobject<NSURLResponse> response(
        [[NSURLResponse alloc] initWithURL:self.request.URL
                                  MIMEType:base::SysUTF8ToNSString(mime_type)
                     expectedContentLength:size
                          textEncodingName:nil]);
    [[self client] URLProtocol:self
            didReceiveResponse:response
            cacheStoragePolicy:NSURLCacheStorageNotAllowed];
    // Read data.
    char bytes[4089];
    size_t nread = 0;
    while ((nread = protocol_job_->Read(bytes, sizeof(bytes))) > 0) {
      NSData* data = [NSData dataWithBytesNoCopy:bytes
                                          length:nread
                                    freeWhenDone:NO];
      [[self client] URLProtocol:self didLoadData:data];
    }
    // Done.
    [[self client] URLProtocolDidFinishLoading:self];
  });
  if (!protocol_job_->Start()) {
    NSError* error = [NSError errorWithDomain:NSURLErrorDomain
                                         code:NSURLErrorCancelled
                                     userInfo:nil];
    [[self client] URLProtocol:self didFailWithError:error];
    return;
  }
}

- (void)stopLoading {
}

@end
