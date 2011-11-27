/**
 * fast quaternion interpolation by Andy Thomason (athomason@acm.org)
 * adapted by bad3p
 */

#pragma once

#include "fundamentals.h"

inline void quatSlerpDirect(Quaternion* out, Quaternion* a, Quaternion* b, float t)
{
    float adotb = a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
    adotb = adotb < 0.995f ? adotb : 0.995f;
    float even = 2.218480716f + 0.2234036692f * adotb * adotb;
    float odd = 2.441884385f * adotb;
    float mTheta = sqrtf( even - odd ) - sqrt( even + odd ) + 1.570796327f + 0.6391287330f * adotb;
    float mRecipSqrt = 1.0f / sqrtf( 1 - adotb * adotb );
    float A = ( 1 - t ) * mTheta - 1.570796327f; A = A * A;
    float B = t * mTheta - 1.570796327f; B = B * B;
    float sinA = 0.9999999535f + ( -0.4999990537f +( 0.4166358517e-1f + ( -0.1385370794e-2f + 0.2315401401e-4f * A ) * A ) * A ) * A;
    float sinB = 0.9999999535f + ( -0.4999990537f +( 0.4166358517e-1f + ( -0.1385370794e-2f + 0.2315401401e-4f * B ) * B ) * B ) * B;
    float alpha = sinA * mRecipSqrt;
    float beta  = sinB * mRecipSqrt;
    out->x = alpha * a->x + beta * b->x;
    out->y = alpha * a->y + beta * b->y;
    out->z = alpha * a->z + beta * b->z;
    out->w = alpha * a->w + beta * b->w;
}

inline void quatSlerpMatrix(Quaternion* out, Quaternion* a, Quaternion* b, float t)
{
    float adotb = a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
    float mRecipOnePlusAdotB = 1.0f / ( 1.0f + adotb );
    float mC1 = 1.570994357f + ( 0.5642929859f + ( -0.1783657717f + 0.4319949352e-1f * adotb ) * adotb ) * adotb;
    float mC3 = -0.6461396382f + ( 0.5945657936f + ( 0.8610323953e-1f - 0.3465122928e-1f * adotb ) * adotb ) * adotb;
    float mC5 = 0.7949823521e-1f + ( -0.1730436931f + ( 0.1079279599f - 0.1439397801e-1f * adotb ) * adotb ) * adotb;
    float mC7 = -0.4354102836e-2f + ( 0.1418962736e-1f + ( -0.1567189691e-1f + 0.5848706227e-2f * adotb ) * adotb ) * adotb;
    float T = 1 - t, t2 = t * t, T2 = T * T;
    float alpha = ( mC1 + ( mC3 + ( mC5 + mC7 * T2 ) * T2 ) * T2 ) * T * mRecipOnePlusAdotB;
    float beta = ( mC1 + ( mC3 + ( mC5 + mC7 * t2 ) * t2 ) * t2 ) * t * mRecipOnePlusAdotB;
    out->x = alpha * a->x + beta * b->x;
    out->y = alpha * a->y + beta * b->y;
    out->z = alpha * a->z + beta * b->z;
    out->w = alpha * a->w + beta * b->w;
}

inline void quatSlerpSimpleRenormal(Quaternion* out, Quaternion* a, Quaternion* b, float t)
{
    float alpha = 1 - t;
    float beta  = t;
    out->x = alpha * a->x + beta * b->x;
    out->y = alpha * a->y + beta * b->y;
    out->z = alpha * a->z + beta * b->z;
    out->w = alpha * a->w + beta * b->w;    
    float dotOut = out->x * out->x + out->y * out->y + out->z * out->z + out->w * out->w;
    float recip = 1.0f / sqrtf( dotOut );
    out->x *= recip;
    out->y *= recip;
    out->z *= recip;
    out->w *= recip;
}