// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/geometry/cubic_bezier.h"

#include <algorithm>
#include <cmath>

#include "base/logging.h"

namespace gfx {

static const double kBezierEpsilon = 1e-7;

CubicBezier::CubicBezier(double p1x, double p1y, double p2x, double p2y) {
  InitCoefficients(p1x, p1y, p2x, p2y);
  InitGradients(p1x, p1y, p2x, p2y);
  InitRange(p1y, p2y);
}

CubicBezier::CubicBezier(const CubicBezier& other) = default;

void CubicBezier::InitCoefficients(double p1x,
                                   double p1y,
                                   double p2x,
                                   double p2y) {
  // Calculate the polynomial coefficients, implicit first and last control
  // points are (0,0) and (1,1).
  cx_ = 3.0 * p1x;
  bx_ = 3.0 * (p2x - p1x) - cx_;
  ax_ = 1.0 - cx_ - bx_;

  cy_ = 3.0 * p1y;
  by_ = 3.0 * (p2y - p1y) - cy_;
  ay_ = 1.0 - cy_ - by_;
}

void CubicBezier::InitGradients(double p1x,
                                double p1y,
                                double p2x,
                                double p2y) {
  // End-point gradients are used to calculate timing function results
  // outside the range [0, 1].
  //
  // There are three possibilities for the gradient at each end:
  // (1) the closest control point is not horizontally coincident with regard to
  //     (0, 0) or (1, 1). In this case the line between the end point and
  //     the control point is tangent to the bezier at the end point.
  // (2) the closest control point is coincident with the end point. In
  //     this case the line between the end point and the far control
  //     point is tangent to the bezier at the end point.
  // (3) the closest control point is horizontally coincident with the end
  //     point, but vertically distinct. In this case the gradient at the
  //     end point is Infinite. However, this causes issues when
  //     interpolating. As a result, we break down to a simple case of
  //     0 gradient under these conditions.

  if (p1x > 0)
    start_gradient_ = p1y / p1x;
  else if (!p1y && p2x > 0)
    start_gradient_ = p2y / p2x;
  else
    start_gradient_ = 0;

  if (p2x < 1)
    end_gradient_ = (p2y - 1) / (p2x - 1);
  else if (p2x == 1 && p1x < 1)
    end_gradient_ = (p1y - 1) / (p1x - 1);
  else
    end_gradient_ = 0;
}

void CubicBezier::InitRange(double p1y, double p2y) {
  range_min_ = 0;
  range_max_ = 1;
  if (0 <= p1y && p1y < 1 && 0 <= p2y && p2y <= 1)
    return;

  const double epsilon = kBezierEpsilon;

  // Represent the function's derivative in the form at^2 + bt + c
  // as in sampleCurveDerivativeY.
  // (Technically this is (dy/dt)*(1/3), which is suitable for finding zeros
  // but does not actually give the slope of the curve.)
  const double a = 3.0 * ay_;
  const double b = 2.0 * by_;
  const double c = cy_;

  // Check if the derivative is constant.
  if (std::abs(a) < epsilon && std::abs(b) < epsilon)
    return;

  // Zeros of the function's derivative.
  double t1 = 0;
  double t2 = 0;

  if (std::abs(a) < epsilon) {
    // The function's derivative is linear.
    t1 = -c / b;
  } else {
    // The function's derivative is a quadratic. We find the zeros of this
    // quadratic using the quadratic formula.
    double discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
      return;
    double discriminant_sqrt = sqrt(discriminant);
    t1 = (-b + discriminant_sqrt) / (2 * a);
    t2 = (-b - discriminant_sqrt) / (2 * a);
  }

  double sol1 = 0;
  double sol2 = 0;

  if (0 < t1 && t1 < 1)
    sol1 = SampleCurveY(t1);

  if (0 < t2 && t2 < 1)
    sol2 = SampleCurveY(t2);

  range_min_ = std::min(std::min(range_min_, sol1), sol2);
  range_max_ = std::max(std::max(range_max_, sol1), sol2);
}

double CubicBezier::SolveCurveX(double x, double epsilon) const {
  DCHECK_GE(x, 0.0);
  DCHECK_LE(x, 1.0);

  double t0;
  double t1;
  double t2;
  double x2;
  double d2;
  int i;

  // First try a few iterations of Newton's method -- normally very fast.
  for (t2 = x, i = 0; i < 8; i++) {
    x2 = SampleCurveX(t2) - x;
    if (fabs(x2) < epsilon)
      return t2;
    d2 = SampleCurveDerivativeX(t2);
    if (fabs(d2) < 1e-6)
      break;
    t2 = t2 - x2 / d2;
  }

  // Fall back to the bisection method for reliability.
  t0 = 0.0;
  t1 = 1.0;
  t2 = x;

  while (t0 < t1) {
    x2 = SampleCurveX(t2);
    if (fabs(x2 - x) < epsilon)
      return t2;
    if (x > x2)
      t0 = t2;
    else
      t1 = t2;
    t2 = (t1 - t0) * .5 + t0;
  }

  // Failure.
  return t2;
}

double CubicBezier::Solve(double x) const {
  return SolveWithEpsilon(x, kBezierEpsilon);
}

double CubicBezier::SlopeWithEpsilon(double x, double epsilon) const {
  x = std::min(std::max(x, 0.0), 1.0);
  double t = SolveCurveX(x, epsilon);
  double dx = SampleCurveDerivativeX(t);
  double dy = SampleCurveDerivativeY(t);
  return dy / dx;
}

double CubicBezier::Slope(double x) const {
  return SlopeWithEpsilon(x, kBezierEpsilon);
}

}  // namespace gfx
