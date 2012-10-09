/* modified jeanclaude.iehl@free.fr */

/*
 * pbrt source code Copyright(c) 1998-2005 Matt Pharr and Greg Humphreys
 *
 * All Rights Reserved.
 * For educational use only; commercial use expressly forbidden.
 * NO WARRANTY, express or implied, for this software.
 * (See file License.txt for complete license)
 */

#ifndef PBRT_TRANSFORM_H
#define PBRT_TRANSFORM_H

#include <iostream>
#include <cstdio>
#include <cassert>

#include "Geometry.h"

namespace gk {

// Matrix4x4 Declarations
//! representation d'une matrice homogene 4x4.
struct Matrix4x4
{
    // Matrix4x4 Public Methods
    //! construit une matrice identite, par defaut.
    Matrix4x4( )
    {
        for(int i = 0; i < 4; ++i)
            for(int j = 0; j < 4; ++j)
                m[i][j] = 0.f;
        
        for(int k= 0; k < 4; k++)
            m[k][k]= 1.f;
    }
    
    //! construit une matrice a partir d'un tableau 2d de reels [ligne][colonne].
    Matrix4x4( const float mat[4][4] );
    
    //! construit une matrice a partir des 16 elements.
    Matrix4x4( 
       float t00, float t01, float t02, float t03,
       float t10, float t11, float t12, float t13,
       float t20, float t21, float t22, float t23,
       float t30, float t31, float t32, float t33 );
    
    //! renvoie la matrice transposee.
    Matrix4x4 Transpose( ) const;
    
    //! affiche la matrice.
    void Print( std::ostream &os ) const 
    {
        os << "[ ";
        for (int i = 0; i < 4; ++i) {
            os << "[ ";
            for (int j = 0; j < 4; ++j)  {
                os << m[i][j];
                if (j != 3) os << ", ";
            }
            os << " ] ";
        }
        os << " ] ";
    }
    
    //! affiche la matrice.
    void print( ) const
    {
        #define M44(m, r, c) m[r][c]
        
        printf("% -.8f  % -.8f  % -.8f  % -.8f\n", 
            M44(m, 0, 0), M44(m, 0, 1), M44(m, 0, 2), M44(m, 0, 3));
        printf("% -.8f  % -.8f  % -.8f  % -.8f\n", 
            M44(m, 1, 0), M44(m, 1, 1), M44(m, 1, 2), M44(m, 1, 3));
        printf("% -.8f  % -.8f  % -.8f  % -.8f\n", 
            M44(m, 2, 0), M44(m, 2, 1), M44(m, 2, 2), M44(m, 2, 3));
        printf("% -.8f  % -.8f  % -.8f  % -.8f\n", 
            M44(m, 3, 0), M44(m, 3, 1), M44(m, 3, 2), M44(m, 3, 3));
        printf("\n");
        
        #undef M44
    }
    
    //! produit de 2 matrices : renvoie m1 * m2.
    static 
    Matrix4x4 Mul( const Matrix4x4 &m1, const Matrix4x4 &m2 )
    {
        float r[4][4];
        
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                r[i][j] = 
                    m1.m[i][0] * m2.m[0][j] +
                    m1.m[i][1] * m2.m[1][j] +
                    m1.m[i][2] * m2.m[2][j] +
                    m1.m[i][3] * m2.m[3][j];
        
        return Matrix4x4(r);
    }
    
    static
    Vector Transform( const Matrix4x4& m, const Vector& v )
    {
        const float x = v.x;
        const float y = v.y;
        const float z = v.z;
        
        return Vector( 
            m.m[0][0]*x + m.m[0][1]*y + m.m[0][2]*z,
            m.m[1][0]*x + m.m[1][1]*y + m.m[1][2]*z,
            m.m[2][0]*x + m.m[2][1]*y + m.m[2][2]*z );        
    }
    
    static
    Point Transform( const Matrix4x4& m, const Point& p )
    {
        const float x = p.x;
        const float y = p.y;
        const float z = p.z;
        
        const float xt = m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z + m.m[0][3];
        const float yt = m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z + m.m[1][3];
        const float zt = m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z + m.m[2][3];
        const float wt = m.m[3][0] * x + m.m[3][1] * y + m.m[3][2] * z + m.m[3][3];
        
        assert( wt != 0 );
        if( wt == 1.f ) 
            return Point( xt, yt, zt );
        else
            return Point( xt, yt, zt ) / wt;        
    }

    static
    Normal Transform( const Matrix4x4& m, const Normal& n )
    {
        const float x = n.x;
        const float y = n.y;
        const float z = n.z;
        
        // utilise la transformation inverse ... pas une transformation directe.
        const float tx = m.m[0][0] * x + m.m[1][0] * y + m.m[2][0] * z;
        const float ty = m.m[0][1] * x + m.m[1][1] * y + m.m[2][1] * z;
        const float tz = m.m[0][2] * x + m.m[1][2] * y + m.m[2][2] * z;
        
        return Normal(tx, ty, tz);
    }
    
    
    //! renvoie l'inverse de la matrice.
    Matrix4x4 getInverse( ) const;
    
    //! conversion en float (*)[4]
    operator float *( )
    {
        return (float *) m;
    }
    
    //! conversion en const float (*)[4]
    operator const float *( ) const
    {
        return (const float *) m;
    }
    
    //! elements de la matrice.
    float m[4][4];
};


// Transform Declarations
//! representation d'une transformation == un changement de repere, du repere '1' vers le repere '2'. 
class Transform
{
public:
    // Transform Public Methods
    //! constructeur par defaut, transformation identite.
    Transform( ) {}
    
    //! construction a partir d'une matrice representee par un tableau 2d de reels.
    Transform( float mat[4][4] )
    {
        m= Matrix4x4(mat);
        mInv = m.getInverse();
    }
    
    //! construction a partir d'une matrice.
    Transform( const Matrix4x4& mat )
    {
        m = mat;
        mInv = m.getInverse();
    }
    
    //! construction a partir d'une matrice et de son inverse.
    Transform( const Matrix4x4& mat, const Matrix4x4& minv )
    {
        m = mat;
        mInv = minv;
    }
    
    //! affiche la matrice representant la transformation.
    void print() const
    {
        m.print();
    }
    
    //! reinitialise la transformation
    void identity( )
    {
        *this= Transform();
    }
    
    //! renvoie la transformation sous forme de matrice.
    const Matrix4x4& matrix( ) const
    {
        return m;
    }
    
    Matrix4x4 transposeMatrix( ) const
    {
        return m.Transpose();
    }
    
    //! renvoie la transformation inverse sous forme de matrice.
    const Matrix4x4& inverseMatrix( ) const
    {
        return mInv;
    }
    
    //! renvoie la matrice de transformation des normales associee a la transformation directe = inverse transpose.
    Matrix4x4 normalMatrix( ) const
    {
        return mInv.Transpose();
    }
    
    //! renvoie la transformation inverse.
    Transform getInverse() const
    {
        return Transform( mInv, m );
    }
    
    //! \name transformations de points, vecteurs, normales, rayons, aabox. passage du repere '1' au repere '2'.
    // @{
    inline Point operator()( const Point &p ) const;
    inline void operator()( const Point &p, Point &pt ) const;

    inline Vector operator()( const Vector &v ) const;
    inline void operator()( const Vector &v, Vector &vt ) const;
    inline Normal operator()( const Normal & ) const;
    inline void operator()( const Normal &, Normal &nt ) const;
    // @}

    //! \name transformations inverses de points, vecteurs, normales, rayons, aabox. passage du repere '2' vers le repere '1'.
    // @{
    inline Point inverse( const Point &p ) const;
    inline void inverse( const Point &p, Point &pt ) const;

    inline Vector inverse( const Vector &v ) const;
    inline void inverse( const Vector &v, Vector &vt ) const;
    inline Normal inverse( const Normal & ) const;
    inline void inverse( const Normal &, Normal &nt ) const;
    // @}
    
    //! composition de 2 transformations.
    Transform operator*( const Transform &t2 ) const;
    
    bool SwapsHandedness() const;

protected:
    // Transform Private Data
    //! les matrices directe et inverse de changement de repere.
    Matrix4x4 m, mInv;
};

Transform Viewport( float width, float height );
Transform Perspective( float fov, float aspect, float znear, float zfar );
Transform Orthographic( float znear, float zfar );
Transform Orthographic( const float left, const float right, const float bottom, const float top, const float znear, const float zfar );
Transform LookAt( const Point &pos, const Point &look, const Vector &up );
Transform Rotate( float angle, const Vector &axis );
Transform RotateX( float angle );
Transform RotateY( float angle );
Transform RotateZ( float angle );
Transform Scale( float x, float y, float z );
Transform Scale( float value );
Transform Translate( const Vector &delta );


// Transform Inline Functions
inline 
Point Transform::operator()( const Point &p ) const
{
    const float x = p.x;
    const float y = p.y;
    const float z = p.z;
    
    const float xt = m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z + m.m[0][3];
    const float yt = m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z + m.m[1][3];
    const float zt = m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z + m.m[2][3];
    const float wt = m.m[3][0] * x + m.m[3][1] * y + m.m[3][2] * z + m.m[3][3];
    
    assert( wt != 0 );
    if( wt == 1.f ) 
        return Point( xt, yt, zt );
    else
        return Point( xt, yt, zt ) / wt;
}


inline 
void Transform::operator()( const Point &p, Point &pt ) const
{
    const float x = p.x;
    const float y = p.y;
    const float z = p.z;
    
    pt.x = m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z + m.m[0][3];
    pt.y = m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z + m.m[1][3];
    pt.z = m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z + m.m[2][3];
    
    const float wt = m.m[3][0] * x + m.m[3][1] * y + m.m[3][2] * z + m.m[3][3];
    assert( wt != 0 );
    if( wt != 1.f ) 
        pt /= wt;
}

inline 
Vector Transform::operator()( const Vector &v ) const
{
    const float x = v.x;
    const float y = v.y;
    const float z = v.z;
    
    return Vector( 
        m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z,
        m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z,
        m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z);
}

inline 
void Transform::operator()( const Vector &v, Vector &vt ) const
{
    const float x = v.x;
    const float y = v.y;
    const float z = v.z;
    
    vt.x = m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z;
    vt.y = m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z;
    vt.z = m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z;
}

inline 
Normal Transform::operator()( const Normal &n ) const
{
    const float x = n.x;
    const float y = n.y;
    const float z = n.z;
    
    return Normal( 
        mInv.m[0][0] * x + mInv.m[1][0] * y + mInv.m[2][0] * z,
        mInv.m[0][1] * x + mInv.m[1][1] * y + mInv.m[2][1] * z,
        mInv.m[0][2] * x + mInv.m[1][2] * y + mInv.m[2][2] * z);
}

inline 
void Transform::operator()( const Normal &n, Normal& nt ) const
{
    const float x = n.x;
    const float y = n.y;
    const float z = n.z;
    
    nt.x = mInv.m[0][0] * x + mInv.m[1][0] * y + mInv.m[2][0] * z;
    nt.y = mInv.m[0][1] * x + mInv.m[1][1] * y + mInv.m[2][1] * z;
    nt.z = mInv.m[0][2] * x + mInv.m[1][2] * y + mInv.m[2][2] * z;
}

// inverse Transform Inline Functions
inline 
Point Transform::inverse( const Point &p ) const
{
    const float x = p.x;
    const float y = p.y;
    const float z = p.z;
    
    const float xt = mInv.m[0][0] * x + mInv.m[0][1] * y + mInv.m[0][2] * z + mInv.m[0][3];
    const float yt = mInv.m[1][0] * x + mInv.m[1][1] * y + mInv.m[1][2] * z + mInv.m[1][3];
    const float zt = mInv.m[2][0] * x + mInv.m[2][1] * y + mInv.m[2][2] * z + mInv.m[2][3];
    const float wt = mInv.m[3][0] * x + mInv.m[3][1] * y + mInv.m[3][2] * z + mInv.m[3][3];
    
    assert( wt != 0 );
    if( wt == 1.f ) 
        return Point( xt, yt, zt );
    else
        return Point( xt, yt, zt ) / wt;
}


inline 
void Transform::inverse( const Point &p, Point &pt ) const
{
    const float x = p.x;
    const float y = p.y;
    const float z = p.z;
    
    pt.x = mInv.m[0][0] * x + mInv.m[0][1] * y + mInv.m[0][2] * z + mInv.m[0][3];
    pt.y = mInv.m[1][0] * x + mInv.m[1][1] * y + mInv.m[1][2] * z + mInv.m[1][3];
    pt.z = mInv.m[2][0] * x + mInv.m[2][1] * y + mInv.m[2][2] * z + mInv.m[2][3];
    
    const float wt = mInv.m[3][0] * x + mInv.m[3][1] * y + mInv.m[3][2] * z + mInv.m[3][3];
    assert( wt != 0 );
    if( wt != 1.f ) 
        pt /= wt;
}

inline 
Vector Transform::inverse( const Vector &v ) const
{
    const float x = v.x;
    const float y = v.y;
    const float z = v.z;
    
    return Vector( 
        mInv.m[0][0] * x + mInv.m[0][1] * y + mInv.m[0][2] * z,
        mInv.m[1][0] * x + mInv.m[1][1] * y + mInv.m[1][2] * z,
        mInv.m[2][0] * x + mInv.m[2][1] * y + mInv.m[2][2] * z);
}

inline 
void Transform::inverse( const Vector &v, Vector &vt ) const
{
    const float x = v.x;
    const float y = v.y;
    const float z = v.z;
    
    vt.x = mInv.m[0][0] * x + mInv.m[0][1] * y + mInv.m[0][2] * z;
    vt.y = mInv.m[1][0] * x + mInv.m[1][1] * y + mInv.m[1][2] * z;
    vt.z = mInv.m[2][0] * x + mInv.m[2][1] * y + mInv.m[2][2] * z;
}

inline 
Normal Transform::inverse( const Normal &n ) const
{
    const float x = n.x;
    const float y = n.y;
    const float z = n.z;
    
    return Normal( 
        m.m[0][0] * x + m.m[1][0] * y + m.m[2][0] * z,
        m.m[0][1] * x + m.m[1][1] * y + m.m[2][1] * z,
        m.m[0][2] * x + m.m[1][2] * y + m.m[2][2] * z);
}

inline 
void Transform::inverse( const Normal &n, Normal& nt ) const
{
    const float x = n.x;
    const float y = n.y;
    const float z = n.z;
    
    nt.x = m.m[0][0] * x + m.m[1][0] * y + m.m[2][0] * z;
    nt.y = m.m[0][1] * x + m.m[1][1] * y + m.m[2][1] * z;
    nt.z = m.m[0][2] * x + m.m[1][2] * y + m.m[2][2] * z;
}

} // namespace

#endif // PBRT_TRANSFORM_H
