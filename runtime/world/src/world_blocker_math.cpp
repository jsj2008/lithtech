//////////////////////////////////////////////////////////////////////////////
// Math routines used by the blocker polygon implementation.  These routines are 
// adapted from the free portion of the WildMagic engine by Magic Software, Inc.  
// The copyright notice on the original implementation is as follows:

// Magic Software, Inc.
// http://www.magic-software.com
// Copyright (c) 2000, 2001.  All Rights Reserved
//
// Source code from Magic Software is supplied under the terms of a license
// agreement and may not be copied or disclosed except in accordance with the
// terms of that agreement.  The various license agreements may be found at
// the Magic Software web site.  This file is subject to the license
//
// FREE SOURCE CODE
// http://www.magic-software.com/License/free.pdf

#include "bdefs.h"

#include "world_blocker_math.h"

float DistSqrSegSeg(const SBlockerSeg& rkSeg0, const SBlockerSeg& rkSeg1,
    float* pfSegP0, float* pfSegP1)
{
    LTVector kDiff = rkSeg0.m_vOrigin - rkSeg1.m_vOrigin;
    float fA00 = rkSeg0.m_vDirection.MagSqr();
    float fA01 = -rkSeg0.m_vDirection.Dot(rkSeg1.m_vDirection);
    float fA11 = rkSeg1.m_vDirection.MagSqr();
    float fB0 = kDiff.Dot(rkSeg0.m_vDirection);
    float fC = kDiff.MagSqr();
    float fDet = fabsf(fA00*fA11-fA01*fA01);
    float fB1, fS, fT, fSqrDist, fTmp;

    if ( fDet >= 0.0001f )
    {
        // line segments are not parallel
        fB1 = -kDiff.Dot(rkSeg1.m_vDirection);
        fS = fA01*fB1-fA11*fB0;
        fT = fA01*fB0-fA00*fB1;
        
        if ( fS >= 0.0 )
        {
            if ( fS <= fDet )
            {
                if ( fT >= 0.0 )
                {
                    if ( fT <= fDet )  // region 0 (interior)
                    {
                        // minimum at two interior points of 3D lines
                        float fInvDet = 1.0f/fDet;
                        fS *= fInvDet;
                        fT *= fInvDet;
                        fSqrDist = fS*(fA00*fS+fA01*fT+2.0f*fB0) +
                            fT*(fA01*fS+fA11*fT+2.0f*fB1)+fC;
                    }
                    else  // region 3 (side)
                    {
                        fT = 1.0;
                        fTmp = fA01+fB0;
                        if ( fTmp >= 0.0 )
                        {
                            fS = 0.0f;
                            fSqrDist = fA11+2.0f*fB1+fC;
                        }
                        else if ( -fTmp >= fA00 )
                        {
                            fS = 1.0f;
                            fSqrDist = fA00+fA11+fC+2.0f*(fB1+fTmp);
                        }
                        else
                        {
                            fS = -fTmp/fA00;
                            fSqrDist = fTmp*fS+fA11+2.0f*fB1+fC;
                        }
                    }
                }
                else  // region 7 (side)
                {
                    fT = 0.0;
                    if ( fB0 >= 0.0 )
                    {
                        fS = 0.0;
                        fSqrDist = fC;
                    }
                    else if ( -fB0 >= fA00 )
                    {
                        fS = 1.0f;
                        fSqrDist = fA00+2.0f*fB0+fC;
                    }
                    else
                    {
                        fS = -fB0/fA00;
                        fSqrDist = fB0*fS+fC;
                    }
                }
            }
            else
            {
                if ( fT >= 0.0 )
                {
                    if ( fT <= fDet )  // region 1 (side)
                    {
                        fS = 1.0;
                        fTmp = fA01+fB1;
                        if ( fTmp >= 0.0 )
                        {
                            fT = 0.0;
                            fSqrDist = fA00+2.0f*fB0+fC;
                        }
                        else if ( -fTmp >= fA11 )
                        {
                            fT = 1.0;
                            fSqrDist = fA00+fA11+fC+2.0f*(fB0+fTmp);
                        }
                        else
                        {
                            fT = -fTmp/fA11;
                            fSqrDist = fTmp*fT+fA00+2.0f*fB0+fC;
                        }
                    }
                    else  // region 2 (corner)
                    {
                        fTmp = fA01+fB0;
                        if ( -fTmp <= fA00 )
                        {
                            fT = 1.0;
                            if ( fTmp >= 0.0 )
                            {
                                fS = 0.0;
                                fSqrDist = fA11+2.0f*fB1+fC;
                            }
                            else
                            {
                                 fS = -fTmp/fA00;
                                 fSqrDist = fTmp*fS+fA11+2.0f*fB1+fC;
                            }
                        }
                        else
                        {
                            fS = 1.0;
                            fTmp = fA01+fB1;
                            if ( fTmp >= 0.0 )
                            {
                                fT = 0.0;
                                fSqrDist = fA00+2.0f*fB0+fC;
                            }
                            else if ( -fTmp >= fA11 )
                            {
                                fT = 1.0;
                                fSqrDist = fA00+fA11+fC+2.0f*(fB0+fTmp);
                            }
                            else
                            {
                                fT = -fTmp/fA11;
                                fSqrDist = fTmp*fT+fA00+2.0f*fB0+fC;
                            }
                        }
                    }
                }
                else  // region 8 (corner)
                {
                    if ( -fB0 < fA00 )
                    {
                        fT = 0.0;
                        if ( fB0 >= 0.0 )
                        {
                            fS = 0.0;
                            fSqrDist = fC;
                        }
                        else
                        {
                            fS = -fB0/fA00;
                            fSqrDist = fB0*fS+fC;
                        }
                    }
                    else
                    {
                        fS = 1.0;
                        fTmp = fA01+fB1;
                        if ( fTmp >= 0.0 )
                        {
                            fT = 0.0;
                            fSqrDist = fA00+2.0f*fB0+fC;
                        }
                        else if ( -fTmp >= fA11 )
                        {
                            fT = 1.0;
                            fSqrDist = fA00+fA11+fC+2.0f*(fB0+fTmp);
                        }
                        else
                        {
                            fT = -fTmp/fA11;
                            fSqrDist = fTmp*fT+fA00+2.0f*fB0+fC;
                        }
                    }
                }
            }
        }
        else 
        {
            if ( fT >= 0.0 )
            {
                if ( fT <= fDet )  // region 5 (side)
                {
                    fS = 0.0;
                    if ( fB1 >= 0.0 )
                    {
                        fT = 0.0;
                        fSqrDist = fC;
                    }
                    else if ( -fB1 >= fA11 )
                    {
                        fT = 1.0;
                        fSqrDist = fA11+2.0f*fB1+fC;
                    }
                    else
                    {
                        fT = -fB1/fA11;
                        fSqrDist = fB1*fT+fC;
                    }
                }
                else  // region 4 (corner)
                {
                    fTmp = fA01+fB0;
                    if ( fTmp < 0.0 )
                    {
                        fT = 1.0;
                        if ( -fTmp >= fA00 )
                        {
                            fS = 1.0;
                            fSqrDist = fA00+fA11+fC+2.0f*(fB1+fTmp);
                        }
                        else
                        {
                            fS = -fTmp/fA00;
                            fSqrDist = fTmp*fS+fA11+2.0f*fB1+fC;
                        }
                    }
                    else
                    {
                        fS = 0.0;
                        if ( fB1 >= 0.0 )
                        {
                            fT = 0.0;
                            fSqrDist = fC;
                        }
                        else if ( -fB1 >= fA11 )
                        {
                            fT = 1.0;
                            fSqrDist = fA11+2.0f*fB1+fC;
                        }
                        else
                        {
                            fT = -fB1/fA11;
                            fSqrDist = fB1*fT+fC;
                        }
                    }
                }
            }
            else   // region 6 (corner)
            {
                if ( fB0 < 0.0 )
                {
                    fT = 0.0;
                    if ( -fB0 >= fA00 )
                    {
                        fS = 1.0;
                        fSqrDist = fA00+2.0f*fB0+fC;
                    }
                    else
                    {
                        fS = -fB0/fA00;
                        fSqrDist = fB0*fS+fC;
                    }
                }
                else
                {
                    fS = 0.0;
                    if ( fB1 >= 0.0 )
                    {
                        fT = 0.0;
                        fSqrDist = fC;
                    }
                    else if ( -fB1 >= fA11 )
                    {
                        fT = 1.0;
                        fSqrDist = fA11+2.0f*fB1+fC;
                    }
                    else
                    {
                        fT = -fB1/fA11;
                        fSqrDist = fB1*fT+fC;
                    }
                }
            }
        }
    }
    else
    {
        // line segments are parallel
        if ( fA01 > 0.0 )
        {
            // direction vectors form an obtuse angle
            if ( fB0 >= 0.0 )
            {
                fS = 0.0;
                fT = 0.0;
                fSqrDist = fC;
            }
            else if ( -fB0 <= fA00 )
            {
                fS = -fB0/fA00;
                fT = 0.0;
                fSqrDist = fB0*fS+fC;
            }
            else
            {
                fB1 = -kDiff.Dot(rkSeg1.m_vDirection);
                fS = 1.0;
                fTmp = fA00+fB0;
                if ( -fTmp >= fA01 )
                {
                    fT = 1.0;
                    fSqrDist = fA00+fA11+fC+2.0f*(fA01+fB0+fB1);
                }
                else
                {
                    fT = -fTmp/fA01;
                    fSqrDist = fA00+2.0f*fB0+fC+fT*(fA11*fT+2.0f*(fA01+fB1));
                }
            }
        }
        else
        {
            // direction vectors form an acute angle
            if ( -fB0 >= fA00 )
            {
                fS = 1.0;
                fT = 0.0;
                fSqrDist = fA00+2.0f*fB0+fC;
            }
            else if ( fB0 <= 0.0 )
            {
                fS = -fB0/fA00;
                fT = 0.0;
                fSqrDist = fB0*fS+fC;
            }
            else
            {
                fB1 = -kDiff.Dot(rkSeg1.m_vDirection);
                fS = 0.0;
                if ( fB0 >= -fA01 )
                {
                    fT = 1.0;
                    fSqrDist = fA11+2.0f*fB1+fC;
                }
                else
                {
                    fT = -fB0/fA01;
                    fSqrDist = fC+fT*(2.0f*fB1+fA11*fT);
                }
            }
        }
    }

    if ( pfSegP0 )
        *pfSegP0 = fS;

    if ( pfSegP1 )
        *pfSegP1 = fT;

    return fabsf(fSqrDist);
}

float DistSqrPtPgm(const LTVector& rkPoint,
    const SBlockerTri& rkPgm, float* pfSParam, float* pfTParam)
{
    LTVector kDiff = rkPgm.m_vOrigin - rkPoint;
    float fA00 = rkPgm.m_vEdge0.MagSqr();
    float fA01 = rkPgm.m_vEdge0.Dot(rkPgm.m_vEdge1);
    float fA11 = rkPgm.m_vEdge1.MagSqr();
    float fB0 = kDiff.Dot(rkPgm.m_vEdge0);
    float fB1 = kDiff.Dot(rkPgm.m_vEdge1);
    float fC = kDiff.MagSqr();
    float fDet = fabsf(fA00*fA11-fA01*fA01);
    float fS = fA01*fB1-fA11*fB0;
    float fT = fA01*fB0-fA00*fB1;
    float fSqrDist, fTmp;

    if ( fS < 0.0 )
    {
        if ( fT < 0.0 )  // region 6
        {
            if ( fB0 < 0.0 )
            {
                fT = 0.0;
                if ( -fB0 >= fA00 )
                {
                    fS = 1.0;
                    fSqrDist = fA00+2.0f*fB0+fC;
                }
                else
                {
                    fS = -fB0/fA00;
                    fSqrDist = fB0*fS+fC;
                }
            }
            else
            {
                fS = 0.0;
                if ( fB1 >= 0.0 )
                {
                    fT = 0.0;
                    fSqrDist = fC;
                }
                else if ( -fB1 >= fA11 )
                {
                    fT = 1.0;
                    fSqrDist = fA11+2.0f*fB1+fC;
                }
                else
                {
                    fT = -fB1/fA11;
                    fSqrDist = fB1*fT+fC;
                }
            }
        }
        else if ( fT <= fDet )  // region 5
        {
            fS = 0.0;
            if ( fB1 >= 0.0 )
            {
                fT = 0.0;
                fSqrDist = fC;
            }
            else if ( -fB1 >= fA11 )
            {
                fT = 1.0;
                fSqrDist = fA11+2.0f*fB1+fC;
            }
            else
            {
                fT = -fB1/fA11;
                fSqrDist = fB1*fT+fC;
            }
        }
        else  // region 4
        {
            fTmp = fA01+fB0;
            if ( fTmp < 0.0 )
            {
                fT = 1.0;
                if ( -fTmp >= fA00 )
                {
                    fS = 1.0;
                    fSqrDist = fA00+fA11+fC+2.0f*(fA01+fB0+fB1);
                }
                else
                {
                    fS = -fTmp/fA00;
                    fSqrDist = fTmp*fS+fA11+2.0f*fB1+fC;
                }
            }
            else
            {
                fS = 0.0;
                if ( fB1 >= 0.0 )
                {
                    fT = 0.0;
                    fSqrDist = fC;
                }
                else if ( -fB1 >= fA11 )
                {
                    fT = 1.0;
                    fSqrDist = fA11+2*fB1+fC;
                }
                else
                {
                    fT = -fB1/fA11;
                    fSqrDist = fB1*fT+fC;
                }
            }
        }
    }
    else if ( fS <= fDet )
    {
        if ( fT < 0.0 )  // region 7
        {
            fT = 0.0;
            if ( fB0 >= 0.0 )
            {
                fS = 0.0;
                fSqrDist = fC;
            }
            else if ( -fB0 >= fA00 )
            {
                fS = 1.0;
                fSqrDist = fA00+2.0f*fB0+fC;
            }
            else
            {
                fS = -fB0/fA00;
                fSqrDist = fB0*fS+fC;
            }
        }
        else if ( fT <= fDet )  // region 0
        {
            // minimum at interior point
            float fInvDet = 1.0f/fDet;
            fS *= fInvDet;
            fT *= fInvDet;
            fSqrDist = fS*(fA00*fS+fA01*fT+2.0f*fB0) +
                fT*(fA01*fS+fA11*fT+2.0f*fB1)+fC;
        }
        else  // region 3
        {
            fT = 1.0;
            fTmp = fA01+fB0;
            if ( fTmp >= 0.0 )
            {
                fS = 0.0;
                fSqrDist = fA11+2.0f*fB1+fC;
            }
            else if ( -fTmp >= fA00 )
            {
                fS = 1.0;
                fSqrDist = fA00+fA11+fC+2.0f*(fA01+fB0+fB1);
            }
            else
            {
                fS = -fTmp/fA00;
                fSqrDist = fTmp*fS+fA11+2.0f*fB1+fC;
            }
        }
    }
    else
    {
        if ( fT < 0.0 )  // region 8
        {
            if ( -fB0 < fA00 )
            {
                fT = 0.0;
                if ( fB0 >= 0.0 )
                {
                    fS = 0.0;
                    fSqrDist = fC;
                }
                else
                {
                    fS = -fB0/fA00;
                    fSqrDist = fB0*fS+fC;
                }
            }
            else
            {
                fS = 1.0;
                fTmp = fA01+fB1;
                if ( fTmp >= 0.0 )
                {
                    fT = 0.0;
                    fSqrDist = fA00+2.0f*fB0+fC;
                }
                else if ( -fTmp >= fA11 )
                {
                    fT = 1.0;
                    fSqrDist = fA00+fA11+fC+2.0f*(fA01+fB0+fB1);
                }
                else
                {
                    fT = -fTmp/fA11;
                    fSqrDist = fTmp*fT+fA00+2.0f*fB0+fC;
                }
            }
        }
        else if ( fT <= fDet )  // region 1
        {
            fS = 1.0;
            fTmp = fA01+fB1;
            if ( fTmp >= 0.0 )
            {
                fT = 0.0;
                fSqrDist = fA00+2*fB0+fC;
            }
            else if ( -fTmp >= fA11 )
            {
                fT = 1.0;
                fSqrDist = fA00+fA11+fC+2.0f*(fA01+fB0+fB1);
            }
            else
            {
                fT = -fTmp/fA11;
                fSqrDist = fTmp*fT+fA00+2.0f*fB0+fC;
            }
        }
        else  // region 2
        {
            fTmp = fA01+fB0;
            if ( -fTmp < fA00 )
            {
                fT = 1.0;
                if ( fTmp >= 0.0 )
                {
                    fS = 0.0;
                    fSqrDist = fA11+2.0f*fB1+fC;
                }
                else
                {
                    fS = -fTmp/fA00;
                    fSqrDist = fTmp*fS+fA11+2.0f*fB1+fC;
                }
            }
            else
            {
                fS = 1.0;
                fTmp = fA01+fB1;
                if ( fTmp >= 0.0 )
                {
                    fT = 0.0;
                    fSqrDist = fA00+2.0f*fB0+fC;
                }
                else if ( -fTmp >= fA11 )
                {
                    fT = 1.0;
                    fSqrDist = fA00+fA11+fC+2.0f*(fA01+fB0+fB1);
                }
                else
                {
                    fT = -fTmp/fA11;
                    fSqrDist = fTmp*fT+fA00+2.0f*fB0+fC;
                }
            }
        }
    }

    if ( pfSParam )
        *pfSParam = fS;

    if ( pfTParam )
        *pfTParam = fT;

    return fabsf(fSqrDist);
}

float DistSqrSegPgm(const SBlockerSeg &rkSeg, const SBlockerTri &rkPgm, 
	float *pfSegP, float *pfPgmP0, float *pfPgmP1)
{
    LTVector kDiff = rkPgm.m_vOrigin - rkSeg.m_vOrigin;
    float fA00 = rkSeg.m_vDirection.MagSqr();
    float fA01 = -rkSeg.m_vDirection.Dot(rkPgm.m_vEdge0);
    float fA02 = -rkSeg.m_vDirection.Dot(rkPgm.m_vEdge1);
    float fA11 = rkPgm.m_vEdge0.MagSqr();
    float fA12 = rkPgm.m_vEdge0.Dot(rkPgm.m_vEdge1);
    float fA22 = rkPgm.m_vEdge1.MagSqr();
    float fB0  = -kDiff.Dot(rkSeg.m_vDirection);
    float fB1  = kDiff.Dot(rkPgm.m_vEdge0);
    float fB2  = kDiff.Dot(rkPgm.m_vEdge1);
    float fCof00 = fA11*fA22-fA12*fA12;
    float fCof01 = fA02*fA12-fA01*fA22;
    float fCof02 = fA01*fA12-fA02*fA11;
    float fDet = fA00*fCof00+fA01*fCof01+fA02*fCof02;

    SBlockerSeg kSegPgm;
    LTVector kPt;
    float fSqrDist, fSqrDist0, fR, fS, fT, fR0, fS0, fT0;

    if ( fabsf(fDet) >= 0.001f )
    {
        float fCof11 = fA00*fA22-fA02*fA02;
        float fCof12 = fA02*fA01-fA00*fA12;
        float fCof22 = fA00*fA11-fA01*fA01;
        float fInvDet = 1.0f/fDet;
        float fRhs0 = -fB0*fInvDet;
        float fRhs1 = -fB1*fInvDet;
        float fRhs2 = -fB2*fInvDet;

        fR = fCof00*fRhs0+fCof01*fRhs1+fCof02*fRhs2;
        fS = fCof01*fRhs0+fCof11*fRhs1+fCof12*fRhs2;
        fT = fCof02*fRhs0+fCof12*fRhs1+fCof22*fRhs2;

        if ( fR < 0.0 )
        {
            if ( fS < 0.0 )
            {
                if ( fT < 0.0 )  // region 6m
                {
                    // min on face s=0 or t=0 or r=0
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 0.0;
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kSegPgm,&fR0,&fS0);
                    fT0 = 0.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                    fSqrDist0 = DistSqrPtPgm(rkSeg.m_vOrigin,rkPgm,&fS0,
                        &fT0);
                    fR0 = 0.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else if ( fT <= 1.0 )  // region 5m
                {
                    // min on face s=0 or r=0
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 0.0;
                    fSqrDist0 = DistSqrPtPgm(rkSeg.m_vOrigin,rkPgm,&fS0,
                        &fT0);
                    fR0 = 0.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else  // region 4m
                {
                    // min on face s=0 or t=1 or r=0
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 0.0;
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge1;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kSegPgm,&fR0,&fS0);
                    fT0 = 1.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                    fSqrDist0 = DistSqrPtPgm(rkSeg.m_vOrigin,rkPgm,&fS0,
                        &fT0);
                    fR0 = 0.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
            }
            else if ( fS <= 1.0 )
            {
                if ( fT < 0.0 )  // region 7m
                {
                    // min on face t=0 or r=0
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fS);
                    fT = 0.0;
                    fSqrDist0 = DistSqrPtPgm(rkSeg.m_vOrigin,rkPgm,&fS0,
                        &fT0);
                    fR0 = 0.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else if ( fT <= 1.0 )  // region 0m
                {
                    // min on face r=0
                    fSqrDist = DistSqrPtPgm(rkSeg.m_vOrigin,rkPgm,&fS,&fT);
                    fR = 0.0;
                }
                else  // region 3m
                {
                    // min on face t=1 or r=0
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge1;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fS);
                    fT = 1.0;
                    fSqrDist0 = DistSqrPtPgm(rkSeg.m_vOrigin,rkPgm,&fS0,
                        &fT0);
                    fR0 = 0.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
            }
            else
            {
                if ( fT < 0.0 )  // region 8m
                {
                    // min on face s=1 or t=0 or r=0
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge0;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 1.0;
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kSegPgm,&fR0,&fS0);
                    fT0 = 0.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                    fSqrDist0 = DistSqrPtPgm(rkSeg.m_vOrigin,rkPgm,&fS0,
                        &fT0);
                    fR0 = 0.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else if ( fT <= 1.0 )  // region 1m
                {
                    // min on face s=1 or r=0
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge0;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 1.0;
                    fSqrDist0 = DistSqrPtPgm(rkSeg.m_vOrigin,rkPgm,&fS0,
                        &fT0);
                    fR0 = 0.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else  // region 2m
                {
                    // min on face s=1 or t=1 or r=0
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge0;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 1.0;
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge1;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kSegPgm,&fR0,&fS0);
                    fT0 = 1.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                    fSqrDist0 = DistSqrPtPgm(rkSeg.m_vOrigin,rkPgm,&fS0,
                        &fT0);
                    fR0 = 0.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
            }
        }
        else if ( fR <= 1.0 )
        {
            if ( fS < 0.0 )
            {
                if ( fT < 0.0 )  // region 6
                {
                    // min on face s=0 or t=0
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 0.0;
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kSegPgm,&fR0,&fS0);
                    fT0 = 0.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else if ( fT <= 1 )  // region 5
                {
                    // min on face s=0
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 0.0;
                }
                else // region 4
                {
                    // min on face s=0 or t=1
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 0.0;
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge1;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kSegPgm,&fR0,&fS0);
                    fT0 = 1.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
            }
            else if ( fS <= 1.0 )
            {
                if ( fT < 0.0 )  // region 7
                {
                    // min on face t=0
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fS);
                    fT = 0.0;
                }
                else if ( fT <= 1.0 )  // region 0
                {
                    // global minimum is interior
                    fSqrDist = fR*(fA00*fR+fA01*fS+fA02*fT+2.0f*fB0)
                          +fS*(fA01*fR+fA11*fS+fA12*fT+2.0f*fB1)
                          +fT*(fA02*fR+fA12*fS+fA22*fT+2.0f*fB2)
                          +kDiff.MagSqr();
                }
                else  // region 3
                {
                    // min on face t=1
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge1;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fS);
                    fT = 1.0;
                }
            }
            else
            {
                if ( fT < 0.0 )  // region 8
                {
                    // min on face s=1 or t=0
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge0;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 1.0;
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kSegPgm,&fR0,&fS0);
                    fT0 = 0.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else if ( fT <= 1.0 )  // region 1
                {
                    // min on face s=1
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge0;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 1.0;
                }
                else  // region 2
                {
                    // min on face s=1 or t=1
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge0;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 1.0;
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge1;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kSegPgm,&fR0,&fS0);
                    fT0 = 1.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
            }
        }
        else
        {
            if ( fS < 0.0 )
            {
                if ( fT < 0.0 )  // region 6p
                {
                    // min on face s=0 or t=0 or r=1
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 0.0;
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kSegPgm,&fR0,&fS0);
                    fT0 = 0.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                    kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
                    fSqrDist0 = DistSqrPtPgm(kPt,rkPgm,&fS0,&fT0);
                    fR0 = 1.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else if ( fT <= 1.0 )  // region 5p
                {
                    // min on face s=0 or r=1
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 0.0;
                    fSqrDist0 = DistSqrPtPgm(rkSeg.m_vOrigin,rkPgm,&fS0,
                        &fT0);
                    kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
                    fSqrDist0 = DistSqrPtPgm(kPt,rkPgm,&fS0,&fT0);
                    fR0 = 1.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else  // region 4p
                {
                    // min on face s=0 or t=1 or r=1
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 0.0;
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge1;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kSegPgm,&fR0,&fS0);
                    fT0 = 1.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                    kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
                    fSqrDist0 = DistSqrPtPgm(kPt,rkPgm,&fS0,&fT0);
                    fR0 = 1.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
            }
            else if ( fS <= 1.0 )
            {
                if ( fT < 0.0 )  // region 7p
                {
                    // min on face t=0 or r=1
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fS);
                    fT = 0.0;
                    kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
                    fSqrDist0 = DistSqrPtPgm(kPt,rkPgm,&fS0,&fT0);
                    fR0 = 1.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else if ( fT <= 1.0 )  // region 0p
                {
                    // min on face r=1
                    kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
                    fSqrDist = DistSqrPtPgm(kPt,rkPgm,&fS,&fT);
                    fR = 1.0;
                }
                else  // region 3p
                {
                    // min on face t=1 or r=1
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge1;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fS);
                    fT = 1.0;
                    kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
                    fSqrDist0 = DistSqrPtPgm(kPt,rkPgm,&fS0,&fT0);
                    fR0 = 1.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
            }
            else
            {
                if ( fT < 0.0 )  // region 8p
                {
                    // min on face s=1 or t=0 or r=1
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge0;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 1.0;
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kSegPgm,&fR0,&fS0);
                    fT0 = 0.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                    kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
                    fSqrDist0 = DistSqrPtPgm(kPt,rkPgm,&fS0,&fT0);
                    fR0 = 1.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else if ( fT <= 1.0 )  // region 1p
                {
                    // min on face s=1 or r=1
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge0;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 1.0;
                    kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
                    fSqrDist0 = DistSqrPtPgm(kPt,rkPgm,&fS0,&fT0);
                    fR0 = 1.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else  // region 2p
                {
                    // min on face s=1 or t=1 or r=1
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge0;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fT);
                    fS = 1.0;
                    kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge1;
                    kSegPgm.m_vDirection = rkPgm.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kSegPgm,&fR0,&fS0);
                    fT0 = 1.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                    kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
                    fSqrDist0 = DistSqrPtPgm(kPt,rkPgm,&fS0,&fT0);
                    fR0 = 1.0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
            }
        }
    }
    else
    {
        // segment and parallelogram are parallel
        kSegPgm.m_vOrigin = rkPgm.m_vOrigin;
        kSegPgm.m_vDirection = rkPgm.m_vEdge0;
        fSqrDist = DistSqrSegSeg(rkSeg,kSegPgm,&fR,&fS);
        fT = 0.0;

        kSegPgm.m_vDirection = rkPgm.m_vEdge1;
        fSqrDist0 = DistSqrSegSeg(rkSeg,kSegPgm,&fR0,&fT0);
        fS0 = 0.0;
        if ( fSqrDist0 < fSqrDist )
        {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
        }

        kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge1;
        kSegPgm.m_vDirection = rkPgm.m_vEdge0;
        fSqrDist0 = DistSqrSegSeg(rkSeg,kSegPgm,&fR0,&fS0);
        fT0 = 1.0;
        if ( fSqrDist0 < fSqrDist )
        {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
        }

        kSegPgm.m_vOrigin = rkPgm.m_vOrigin+rkPgm.m_vEdge0;
        kSegPgm.m_vDirection = rkPgm.m_vEdge1;
        fSqrDist0 = DistSqrSegSeg(rkSeg,kSegPgm,&fR0,&fT0);
        fS0 = 1.0;
        if ( fSqrDist0 < fSqrDist )
        {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
        }

        fSqrDist0 = DistSqrPtPgm(rkSeg.m_vOrigin,rkPgm,&fS0,&fT0);
        fR0 = 0.0;
        if ( fSqrDist0 < fSqrDist )
        {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
        }

        kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
        fSqrDist0 = DistSqrPtPgm(kPt,rkPgm,&fS0,&fT0);
        fR0 = 1.0;
        if ( fSqrDist0 < fSqrDist )
        {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
        }
    }

    if ( pfSegP )
        *pfSegP = fR;

    if ( pfPgmP0 )
        *pfPgmP0 = fS;

    if ( pfPgmP1 )
        *pfPgmP1 = fT;

    return fabsf(fSqrDist);
}

float DistSqrPtTri(const LTVector& rkPoint, const SBlockerTri& rkTri,
    float* pfSParam, float* pfTParam)
{
    LTVector kDiff = rkTri.m_vOrigin - rkPoint;
    float fA00 = rkTri.m_vEdge0.MagSqr();
    float fA01 = rkTri.m_vEdge0.Dot(rkTri.m_vEdge1);
    float fA11 = rkTri.m_vEdge1.MagSqr();
    float fB0 = kDiff.Dot(rkTri.m_vEdge0);
    float fB1 = kDiff.Dot(rkTri.m_vEdge1);
    float fC = kDiff.MagSqr();
    float fDet = fabsf(fA00*fA11-fA01*fA01);
    float fS = fA01*fB1-fA11*fB0;
    float fT = fA01*fB0-fA00*fB1;
    float fSqrDist;

    if ( fS + fT <= fDet )
    {
        if ( fS < 0.0 )
        {
            if ( fT < 0.0 )  // region 4
            {
                if ( fB0 < 0.0 )
                {
                    fT = 0.0;
                    if ( -fB0 >= fA00 )
                    {
                        fS = 1.0;
                        fSqrDist = fA00+2.0f*fB0+fC;
                    }
                    else
                    {
                        fS = -fB0/fA00;
                        fSqrDist = fB0*fS+fC;
                    }
                }
                else
                {
                    fS = 0.0;
                    if ( fB1 >= 0.0 )
                    {
                        fT = 0.0;
                        fSqrDist = fC;
                    }
                    else if ( -fB1 >= fA11 )
                    {
                        fT = 1.0;
                        fSqrDist = fA11+2.0f*fB1+fC;
                    }
                    else
                    {
                        fT = -fB1/fA11;
                        fSqrDist = fB1*fT+fC;
                    }
                }
            }
            else  // region 3
            {
                fS = 0.0;
                if ( fB1 >= 0.0 )
                {
                    fT = 0.0;
                    fSqrDist = fC;
                }
                else if ( -fB1 >= fA11 )
                {
                    fT = 1;
                    fSqrDist = fA11+2.0f*fB1+fC;
                }
                else
                {
                    fT = -fB1/fA11;
                    fSqrDist = fB1*fT+fC;
                }
            }
        }
        else if ( fT < 0.0 )  // region 5
        {
            fT = 0.0;
            if ( fB0 >= 0.0 )
            {
                fS = 0.0;
                fSqrDist = fC;
            }
            else if ( -fB0 >= fA00 )
            {
                fS = 1.0;
                fSqrDist = fA00+2.0f*fB0+fC;
            }
            else
            {
                fS = -fB0/fA00;
                fSqrDist = fB0*fS+fC;
            }
        }
        else  // region 0
        {
            // minimum at interior point
            float fInvDet = 1.0f/fDet;
            fS *= fInvDet;
            fT *= fInvDet;
            fSqrDist = fS*(fA00*fS+fA01*fT+2.0f*fB0) +
                fT*(fA01*fS+fA11*fT+2.0f*fB1)+fC;
        }
    }
    else
    {
        float fTmp0, fTmp1, fNumer, fDenom;

        if ( fS < 0.0 )  // region 2
        {
            fTmp0 = fA01 + fB0;
            fTmp1 = fA11 + fB1;
            if ( fTmp1 > fTmp0 )
            {
                fNumer = fTmp1 - fTmp0;
                fDenom = fA00-2.0f*fA01+fA11;
                if ( fNumer >= fDenom )
                {
                    fS = 1.0;
                    fT = 0.0;
                    fSqrDist = fA00+2.0f*fB0+fC;
                }
                else
                {
                    fS = fNumer/fDenom;
                    fT = 1.0f - fS;
                    fSqrDist = fS*(fA00*fS+fA01*fT+2.0f*fB0) +
                        fT*(fA01*fS+fA11*fT+2.0f*fB1)+fC;
                }
            }
            else
            {
                fS = 0.0;
                if ( fTmp1 <= 0.0 )
                {
                    fT = 1.0;
                    fSqrDist = fA11+2.0f*fB1+fC;
                }
                else if ( fB1 >= 0.0 )
                {
                    fT = 0.0;
                    fSqrDist = fC;
                }
                else
                {
                    fT = -fB1/fA11;
                    fSqrDist = fB1*fT+fC;
                }
            }
        }
        else if ( fT < 0.0 )  // region 6
        {
            fTmp0 = fA01 + fB1;
            fTmp1 = fA00 + fB0;
            if ( fTmp1 > fTmp0 )
            {
                fNumer = fTmp1 - fTmp0;
                fDenom = fA00-2.0f*fA01+fA11;
                if ( fNumer >= fDenom )
                {
                    fT = 1.0;
                    fS = 0.0;
                    fSqrDist = fA11+2.0f*fB1+fC;
                }
                else
                {
                    fT = fNumer/fDenom;
                    fS = 1.0f - fT;
                    fSqrDist = fS*(fA00*fS+fA01*fT+2.0f*fB0) +
                        fT*(fA01*fS+fA11*fT+2.0f*fB1)+fC;
                }
            }
            else
            {
                fT = 0.0;
                if ( fTmp1 <= 0.0 )
                {
                    fS = 1.0;
                    fSqrDist = fA00+2.0f*fB0+fC;
                }
                else if ( fB0 >= 0.0 )
                {
                    fS = 0.0;
                    fSqrDist = fC;
                }
                else
                {
                    fS = -fB0/fA00;
                    fSqrDist = fB0*fS+fC;
                }
            }
        }
        else  // region 1
        {
            fNumer = fA11 + fB1 - fA01 - fB0;
            if ( fNumer <= 0.0 )
            {
                fS = 0.0;
                fT = 1.0;
                fSqrDist = fA11+2.0f*fB1+fC;
            }
            else
            {
                fDenom = fA00-2.0f*fA01+fA11;
                if ( fNumer >= fDenom )
                {
                    fS = 1.0;
                    fT = 0.0;
                    fSqrDist = fA00+2.0f*fB0+fC;
                }
                else
                {
                    fS = fNumer/fDenom;
                    fT = 1.0f - fS;
                    fSqrDist = fS*(fA00*fS+fA01*fT+2.0f*fB0) +
                        fT*(fA01*fS+fA11*fT+2.0f*fB1)+fC;
                }
            }
        }
    }

    if ( pfSParam )
        *pfSParam = fS;

    if ( pfTParam )
        *pfTParam = fT;

    return fabsf(fSqrDist);
}

float DistSqrSegTri(const SBlockerSeg& rkSeg, const SBlockerTri& rkTri,
    float* pfSegP, float* pfTriP0, float* pfTriP1)
{
    LTVector kDiff = rkTri.m_vOrigin - rkSeg.m_vOrigin;
    float fA00 = rkSeg.m_vDirection.MagSqr();
    float fA01 = -rkSeg.m_vDirection.Dot(rkTri.m_vEdge0);
    float fA02 = -rkSeg.m_vDirection.Dot(rkTri.m_vEdge1);
    float fA11 = rkTri.m_vEdge0.MagSqr();
    float fA12 = rkTri.m_vEdge0.Dot(rkTri.m_vEdge1);
    float fA22 = rkTri.m_vEdge1.Dot(rkTri.m_vEdge1);
    float fB0  = -kDiff.Dot(rkSeg.m_vDirection);
    float fB1  = kDiff.Dot(rkTri.m_vEdge0);
    float fB2  = kDiff.Dot(rkTri.m_vEdge1);
    float fCof00 = fA11*fA22-fA12*fA12;
    float fCof01 = fA02*fA12-fA01*fA22;
    float fCof02 = fA01*fA12-fA02*fA11;
    float fDet = fA00*fCof00+fA01*fCof01+fA02*fCof02;

    SBlockerSeg kTriSeg;
    LTVector kPt;
    float fSqrDist, fSqrDist0, fR, fS, fT, fR0, fS0, fT0;

    if ( fabsf(fDet) >= 0.0001f )
    {
        float fCof11 = fA00*fA22-fA02*fA02;
        float fCof12 = fA02*fA01-fA00*fA12;
        float fCof22 = fA00*fA11-fA01*fA01;
        float fInvDet = 1.0f/fDet;
        float fRhs0 = -fB0*fInvDet;
        float fRhs1 = -fB1*fInvDet;
        float fRhs2 = -fB2*fInvDet;

        fR = fCof00*fRhs0+fCof01*fRhs1+fCof02*fRhs2;
        fS = fCof01*fRhs0+fCof11*fRhs1+fCof12*fRhs2;
        fT = fCof02*fRhs0+fCof12*fRhs1+fCof22*fRhs2;

        if ( fR < 0.0f )
        {
            if ( fS+fT <= 1.0f )
            {
                if ( fS < 0.0f )
                {
                    if ( fT < 0.0f )  // region 4m
                    {
                        // min on face s=0 or t=0 or r=0
                        kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                        kTriSeg.m_vDirection = rkTri.m_vEdge1;
                        fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fT);
                        fS = 0.0f;
                        kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                        kTriSeg.m_vDirection = rkTri.m_vEdge0;
                        fSqrDist0 = DistSqrSegSeg(rkSeg,kTriSeg,&fR0,&fS0);
                        fT0 = 0.0f;
                        if ( fSqrDist0 < fSqrDist )
                        {
                            fSqrDist = fSqrDist0;
                            fR = fR0;
                            fS = fS0;
                            fT = fT0;
                        }
                        fSqrDist0 = DistSqrPtTri(rkSeg.m_vOrigin,rkTri,&fS0,
                            &fT0);
                        fR0 = 0.0f;
                        if ( fSqrDist0 < fSqrDist )
                        {
                            fSqrDist = fSqrDist0;
                            fR = fR0;
                            fS = fS0;
                            fT = fT0;
                        }
                    }
                    else  // region 3m
                    {
                        // min on face s=0 or r=0
                        kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                        kTriSeg.m_vDirection = rkTri.m_vEdge1;
                        fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fT);
                        fS = 0.0f;
                        fSqrDist0 = DistSqrPtTri(rkSeg.m_vOrigin,rkTri,&fS0,
                            &fT0);
                        fR0 = 0.0f;
                        if ( fSqrDist0 < fSqrDist )
                        {
                            fSqrDist = fSqrDist0;
                            fR = fR0;
                            fS = fS0;
                            fT = fT0;
                        }
                    }
                }
                else if ( fT < 0.0f )  // region 5m
                {
                    // min on face t=0 or r=0
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                    kTriSeg.m_vDirection = rkTri.m_vEdge0;
                    fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fS);
                    fT = 0.0f;
                    fSqrDist0 = DistSqrPtTri(rkSeg.m_vOrigin,rkTri,&fS0,&fT0);
                    fR0 = 0.0f;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else  // region 0m
                {
                    // min on face r=0
                    fSqrDist = DistSqrPtTri(rkSeg.m_vOrigin,rkTri,&fS,&fT);
                    fR = 0.0f;
                }
            }
            else
            {
                if ( fS < 0.0f )  // region 2m
                {
                    // min on face s=0 or s+t=1 or r=0
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                    kTriSeg.m_vDirection = rkTri.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fT);
                    fS = 0.0f;
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin+rkTri.m_vEdge0;
                    kTriSeg.m_vDirection = rkTri.m_vEdge1-rkTri.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kTriSeg,&fR0,&fT0);
                    fS0 = 1.0f-fT0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                    fSqrDist0 = DistSqrPtTri(rkSeg.m_vOrigin,rkTri,&fS0,&fT0);
                    fR0 = 0.0f;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else if ( fT < 0.0f )  // region 6m
                {
                    // min on face t=0 or s+t=1 or r=0
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                    kTriSeg.m_vDirection = rkTri.m_vEdge0;
                    fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fS);
                    fT = 0.0f;
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin+rkTri.m_vEdge0;
                    kTriSeg.m_vDirection = rkTri.m_vEdge1-rkTri.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kTriSeg,&fR0,&fT0);
                    fS0 = 1.0f-fT0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                    fSqrDist0 = DistSqrPtTri(rkSeg.m_vOrigin,rkTri,&fS0,&fT0);
                    fR0 = 0.0f;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else  // region 1m
                {
                    // min on face s+t=1 or r=0
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin+rkTri.m_vEdge0;
                    kTriSeg.m_vDirection = rkTri.m_vEdge1-rkTri.m_vEdge0;
                    fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fT);
                    fS = 1.0f-fT;
                    fSqrDist0 = DistSqrPtTri(rkSeg.m_vOrigin,rkTri,&fS0,&fT0);
                    fR0 = 0.0f;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
            }
        }
        else if ( fR <= 1.0f )
        {
            if ( fS+fT <= 1.0f )
            {
                if ( fS < 0.0f )
                {
                    if ( fT < 0.0f )  // region 4
                    {
                        // min on face s=0 or t=0
                        kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                        kTriSeg.m_vDirection = rkTri.m_vEdge1;
                        fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fT);
                        fS = 0.0f;
                        kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                        kTriSeg.m_vDirection = rkTri.m_vEdge0;
                        fSqrDist0 = DistSqrSegSeg(rkSeg,kTriSeg,&fR0,&fS0);
                        fT0 = 0.0f;
                        if ( fSqrDist0 < fSqrDist )
                        {
                            fSqrDist = fSqrDist0;
                            fR = fR0;
                            fS = fS0;
                            fT = fT0;
                        }
                    }
                    else  // region 3
                    {
                        // min on face s=0
                        kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                        kTriSeg.m_vDirection = rkTri.m_vEdge1;
                        fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fT);
                        fS = 0.0f;
                    }
                }
                else if ( fT < 0.0f )  // region 5
                {
                    // min on face t=0
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                    kTriSeg.m_vDirection = rkTri.m_vEdge0;
                    fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fS);
                    fT = 0.0f;
                }
                else  // region 0
                {
                    // global minimum is interior, done
                    fSqrDist = fR*(fA00*fR+fA01*fS+fA02*fT+2.0f*fB0)
                          +fS*(fA01*fR+fA11*fS+fA12*fT+2.0f*fB1)
                          +fT*(fA02*fR+fA12*fS+fA22*fT+2.0f*fB2)
                          +kDiff.MagSqr();
                }
            }
            else
            {
                if ( fS < 0.0f )  // region 2
                {
                    // min on face s=0 or s+t=1
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                    kTriSeg.m_vDirection = rkTri.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fT);
                    fS = 0.0f;
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin+rkTri.m_vEdge0;
                    kTriSeg.m_vDirection = rkTri.m_vEdge1-rkTri.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kTriSeg,&fR0,&fT0);
                    fS0 = 1.0f-fT0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else if ( fT < 0.0f )  // region 6
                {
                    // min on face t=0 or s+t=1
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                    kTriSeg.m_vDirection = rkTri.m_vEdge0;
                    fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fS);
                    fT = 0.0f;
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin+rkTri.m_vEdge0;
                    kTriSeg.m_vDirection = rkTri.m_vEdge1-rkTri.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kTriSeg,&fR0,&fT0);
                    fS0 = 1.0f-fT0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else  // region 1
                {
                    // min on face s+t=1
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin+rkTri.m_vEdge0;
                    kTriSeg.m_vDirection = rkTri.m_vEdge1-rkTri.m_vEdge0;
                    fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fT);
                    fS = 1.0f-fT;
                }
            }
        }
        else  // fR > 1
        {
            if ( fS+fT <= 1.0f )
            {
                if ( fS < 0.0f )
                {
                    if ( fT < 0.0f )  // region 4p
                    {
                        // min on face s=0 or t=0 or r=1
                        kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                        kTriSeg.m_vDirection = rkTri.m_vEdge1;
                        fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fT);
                        fS = 0.0f;
                        kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                        kTriSeg.m_vDirection = rkTri.m_vEdge0;
                        fSqrDist0 = DistSqrSegSeg(rkSeg,kTriSeg,&fR0,&fS0);
                        fT0 = 0.0f;
                        if ( fSqrDist0 < fSqrDist )
                        {
                            fSqrDist = fSqrDist0;
                            fR = fR0;
                            fS = fS0;
                            fT = fT0;
                        }
                        kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
                        fSqrDist0 = DistSqrPtTri(kPt,rkTri,&fS0,&fT0);
                        fR0 = 1.0f;
                        if ( fSqrDist0 < fSqrDist )
                        {
                            fSqrDist = fSqrDist0;
                            fR = fR0;
                            fS = fS0;
                            fT = fT0;
                        }
                    }
                    else  // region 3p
                    {
                        // min on face s=0 or r=1
                        kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                        kTriSeg.m_vDirection = rkTri.m_vEdge1;
                        fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fT);
                        fS = 0.0f;
                        kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
                        fSqrDist0 = DistSqrPtTri(kPt,rkTri,&fS0,&fT0);
                        fR0 = 1.0f;
                        if ( fSqrDist0 < fSqrDist )
                        {
                            fSqrDist = fSqrDist0;
                            fR = fR0;
                            fS = fS0;
                            fT = fT0;
                        }
                    }
                }
                else if ( fT < 0.0f )  // region 5p
                {
                    // min on face t=0 or r=1
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                    kTriSeg.m_vDirection = rkTri.m_vEdge0;
                    fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fS);
                    fT = 0.0f;
                    kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
                    fSqrDist0 = DistSqrPtTri(kPt,rkTri,&fS0,&fT0);
                    fR0 = 1.0f;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else  // region 0p
                {
                    // min face on r=1
                    kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
                    fSqrDist = DistSqrPtTri(kPt,rkTri,&fS,&fT);
                    fR = 1.0f;
                }
            }
            else
            {
                if ( fS < 0.0f )  // region 2p
                {
                    // min on face s=0 or s+t=1 or r=1
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                    kTriSeg.m_vDirection = rkTri.m_vEdge1;
                    fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fT);
                    fS = 0.0f;
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin+rkTri.m_vEdge0;
                    kTriSeg.m_vDirection = rkTri.m_vEdge1-rkTri.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kTriSeg,&fR0,&fT0);
                    fS0 = 1.0f-fT0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                    kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
                    fSqrDist0 = DistSqrPtTri(kPt,rkTri,&fS0,&fT0);
                    fR0 = 1.0f;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else if ( fT < 0.0f )  // region 6p
                {
                    // min on face t=0 or s+t=1 or r=1
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin;
                    kTriSeg.m_vDirection = rkTri.m_vEdge0;
                    fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fS);
                    fT = 0.0f;
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin+rkTri.m_vEdge0;
                    kTriSeg.m_vDirection = rkTri.m_vEdge1-rkTri.m_vEdge0;
                    fSqrDist0 = DistSqrSegSeg(rkSeg,kTriSeg,&fR0,&fT0);
                    fS0 = 1.0f-fT0;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                    kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
                    fSqrDist0 = DistSqrPtTri(kPt,rkTri,&fS0,&fT0);
                    fR0 = 1.0f;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
                else  // region 1p
                {
                    // min on face s+t=1 or r=1
                    kTriSeg.m_vOrigin = rkTri.m_vOrigin+rkTri.m_vEdge0;
                    kTriSeg.m_vDirection = rkTri.m_vEdge1-rkTri.m_vEdge0;
                    fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fT);
                    fS = 1.0f-fT;
                    kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
                    fSqrDist0 = DistSqrPtTri(kPt,rkTri,&fS0,&fT0);
                    fR0 = 1.0f;
                    if ( fSqrDist0 < fSqrDist )
                    {
                        fSqrDist = fSqrDist0;
                        fR = fR0;
                        fS = fS0;
                        fT = fT0;
                    }
                }
            }
        }
    }
    else
    {
        // segment and triangle are parallel
        kTriSeg.m_vOrigin = rkTri.m_vOrigin;
        kTriSeg.m_vDirection = rkTri.m_vEdge0;
        fSqrDist = DistSqrSegSeg(rkSeg,kTriSeg,&fR,&fS);
        fT = 0.0f;

        kTriSeg.m_vDirection = rkTri.m_vEdge1;
        fSqrDist0 = DistSqrSegSeg(rkSeg,kTriSeg,&fR0,&fT0);
        fS0 = 0.0f;
        if ( fSqrDist0 < fSqrDist )
        {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
        }

        kTriSeg.m_vOrigin = rkTri.m_vOrigin + rkTri.m_vEdge0;
        kTriSeg.m_vDirection = rkTri.m_vEdge1 - rkTri.m_vEdge0;
        fSqrDist0 = DistSqrSegSeg(rkSeg,kTriSeg,&fR0,&fT0);
        fS0 = 1.0f-fT0;
        if ( fSqrDist0 < fSqrDist )
        {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
        }

        fSqrDist0 = DistSqrPtTri(rkSeg.m_vOrigin,rkTri,&fS0,&fT0);
        fR0 = 0.0f;
        if ( fSqrDist0 < fSqrDist )
        {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
        }

        kPt = rkSeg.m_vOrigin+rkSeg.m_vDirection;
        fSqrDist0 = DistSqrPtTri(kPt,rkTri,&fS0,&fT0);
        fR0 = 1.0f;
        if ( fSqrDist0 < fSqrDist )
        {
            fSqrDist = fSqrDist0;
            fR = fR0;
            fS = fS0;
            fT = fT0;
        }
    }

    if ( pfSegP )
        *pfSegP = fR;

    if ( pfTriP0 )
        *pfTriP0 = fS;

    if ( pfTriP1 )
        *pfTriP1 = fT;

    return fabsf(fSqrDist);
}

float DistSqrTriPgm(const SBlockerTri &rkTri, const SBlockerTri &rkPgm,
	float *pfTriP0, float *pfTriP1, float *pfPgmP0, float *pfPgmP1)
{
	float fS, fT, fS0, fT0;  // triangle parameters
    float fU, fV, fU0, fV0;  // parallelogram parameters
    float fSqrDist, fSqrDist0;
    SBlockerSeg kSeg;

    // compare edges of tri against all of pgm
    kSeg.m_vOrigin = rkTri.m_vOrigin;
    kSeg.m_vDirection = rkTri.m_vEdge0;
    fSqrDist = DistSqrSegPgm(kSeg,rkPgm,&fS,&fU,&fV);
    fT = 0.0;

    kSeg.m_vDirection = rkTri.m_vEdge1;
    fSqrDist0 = DistSqrSegPgm(kSeg,rkPgm,&fT0,&fU0,&fV0);
    fS0 = 0.0;
    if ( fSqrDist0 < fSqrDist )
    {
        fSqrDist = fSqrDist0;
        fS = fS0;
        fT = fT0;
        fU = fU0;
        fV = fV0;
    }

    kSeg.m_vOrigin = kSeg.m_vOrigin + rkTri.m_vEdge0;
    kSeg.m_vDirection = kSeg.m_vDirection - rkTri.m_vEdge0;
    fSqrDist0 = DistSqrSegPgm(kSeg,rkPgm,&fT0,&fU0,&fV0);
    fS0 = 1.0f-fT0;
    if ( fSqrDist0 < fSqrDist )
    {
        fSqrDist = fSqrDist0;
        fS = fS0;
        fT = fT0;
        fU = fU0;
        fV = fV0;
    }

    // compare edges of pgm against all of tri
    kSeg.m_vOrigin = rkPgm.m_vOrigin;
    kSeg.m_vDirection = rkPgm.m_vEdge0;
    fSqrDist0 = DistSqrSegTri(kSeg,rkTri,&fU0,&fS0,&fT0);
    fV0 = 0.0;
    if ( fSqrDist0 < fSqrDist )
    {
        fSqrDist = fSqrDist0;
        fS = fS0;
        fT = fT0;
        fU = fU0;
        fV = fV0;
    }

    kSeg.m_vDirection = rkPgm.m_vEdge1;
    fSqrDist0 = DistSqrSegTri(kSeg,rkTri,&fV0,&fS0,&fT0);
    fU0 = 0.0;
    if ( fSqrDist0 < fSqrDist )
    {
        fSqrDist = fSqrDist0;
        fS = fS0;
        fT = fT0;
        fU = fU0;
        fV = fV0;
    }

    kSeg.m_vOrigin = rkPgm.m_vOrigin + rkPgm.m_vEdge1;
    kSeg.m_vDirection = rkPgm.m_vEdge0;
    fSqrDist0 = DistSqrSegTri(kSeg,rkTri,&fU0,&fS0,&fT0);
    fV0 = 1.0;
    if ( fSqrDist0 < fSqrDist )
    {
        fSqrDist = fSqrDist0;
        fS = fS0;
        fT = fT0;
        fU = fU0;
        fV = fV0;
    }

    kSeg.m_vOrigin = rkPgm.m_vOrigin + rkPgm.m_vEdge0;
    kSeg.m_vDirection = rkPgm.m_vEdge1;
    fSqrDist0 = DistSqrSegTri(kSeg,rkTri,&fV0,&fS0,&fT0);
    fU0 = 1.0;
    if ( fSqrDist0 < fSqrDist )
    {
        fSqrDist = fSqrDist0;
        fS = fS0;
        fT = fT0;
        fU = fU0;
        fV = fV0;
    }

    if ( pfTriP0 )
        *pfTriP0 = fS;

    if ( pfTriP1 )
        *pfTriP1 = fT;

    if ( pfPgmP0 )
        *pfPgmP0 = fU;

    if ( pfPgmP1 )
        *pfPgmP1 = fV;

    return fabsf(fSqrDist);
}
