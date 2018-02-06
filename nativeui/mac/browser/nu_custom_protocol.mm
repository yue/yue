// Copyright 2018 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/mac/browser/nu_custom_protocol.h"

#include <map>

#include "base/mac/scoped_nsobject.h"
#include "base/strings/sys_string_conversions.h"

namespace {

// Whether we have registered this protocol class.
bool g_initailized = false;

// A map of schemes and handlers.
std::map<std::string, nu::Browser::ProtocolHandler> g_handlers;

}  // namespace

@implementation NUCustomProtocol

+ (bool)registerProtocol:(NSString*)scheme
             withHandler:(nu::Browser::ProtocolHandler)handler {
  if (!g_initailized) {
    [NSURLProtocol registerClass:[NUCustomProtocol class]];
    g_initailized = true;
  }
  g_handlers[[scheme UTF8String]] = std::move(handler);
  // This private API can make WKWebview aware of our custom protocol class.
  Class cls = NSClassFromString(@"WKBrowsingContextController");
  SEL sel = NSSelectorFromString(@"registerSchemeForCustomProtocol:");
  if (![cls respondsToSelector:sel])
    return false;
  [cls performSelector:sel withObject:scheme];
  return true;
}

+ (bool)unregisterProtocol:(NSString*)scheme {
  g_handlers.erase([scheme UTF8String]);
  // The private API to unregister protocol.
  Class cls = NSClassFromString(@"WKBrowsingContextController");
  SEL sel = NSSelectorFromString(@"unregisterSchemeForCustomProtocol:");
  if (![cls respondsToSelector:sel])
    return false;
  [cls performSelector:sel withObject:scheme];
  return true;
}

+ (BOOL)canInitWithRequest:(NSURLRequest*)request {
  return g_handlers.find([request.URL.scheme UTF8String]) != g_handlers.end();
}

+ (NSURLRequest*)canonicalRequestForRequest:(NSURLRequest*)request {
  return request;
}

- (void)startLoading {
  // Create job.
  protocol_job_ = g_handlers[[self.request.URL.scheme UTF8String]](
      [[self.request.URL absoluteString] UTF8String]);
  if (!protocol_job_) {
    NSError* error = [NSError errorWithDomain:NSURLErrorDomain
                                         code:NSURLErrorUnsupportedURL
                                     userInfo:nil];
    [[self client] URLProtocol:self didFailWithError:error];
    return;
  }

  // Start.
  protocol_job_->Plug([&](size_t size) {
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
