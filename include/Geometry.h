/* modified jeanclaude.iehl@free.fr */

/*
 * pbrt source code Copyright(c) 1998-2005 Matt Pharr and Greg Humphreys
 *
 * All Rights Reserved.
 * For educational use only; commercial use expressly forbidden.
 * NO WARRANTY, express or implied, for this software.
 * (See file License.txt for complete license)
 */

#ifndef PBRT_GEOMETRY_H
#define PBRT_GEOMETRY_H

#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cmath>

//! namespace pour regrouper les types et les fonctions.
namespace gk {

#define RAY_EPSILON 0.0001f
#define EPSILON 0.00001f

// Geometry Declarations
class Point;
class Normal;
class Vector;

//! vecteur en dimension 3.
//! represente une direction, cf. Point et Normal pour representer un point et une normale.
class Vector
{
public:
    // Vector Public Methods
    //! constructeur.
    Vector( const float _x = 0.f, const float _y = 0.f, const float _z = 0.f )
        : 
        x( _x ), y( _y ), z( _z )
    {}
    
    //! construit un vecteur a partir des coordonnees d'un point.
    explicit Vector( const Point &p );
    
    //! construit un vecteur a partir des coordonnees d'une normale.
    explicit Vector( const Normal &n );
    
    //! construit le vecteur pq, origine p, direction q - p.
    Vector( const Point& p, const Point& q );
    
    //! affiche un vecteur.
    void print( ) const
    {
        printf("% -.8f % -.8f % -.8f\n", x, y, z);
    }
    
    //! addition de 2 vecteurs, w= u + v, renvoie w.
    Vector operator+( const Vector &v ) const
    {
        return Vector( x + v.x, y + v.y, z + v.z );
    }
    
    //! addition de 2 vecteurs, u= u + v.
    Vector& operator+=( const Vector &v )
    {
        x += v.x;
        y += v.y;
        z += v.z;
        
        return *this;
    }
    
    //! soustraction de 2 vecteurs, w= u - v, renvoie w.
    Vector operator-( const Vector &v ) const
    {
        return Vector( x - v.x, y - v.y, z - v.z );
    }
    
    //! soustraction de 2 vecteurs, u= u - v.
    Vector& operator-=( const Vector &v )
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        
        return *this;
    }
    
    //! comparaison de 2 vecteurs.
    bool operator==( const Vector &v ) const
    {
        return (x == v.x && y == v.y && z == v.z);
    }
    
    //! produit par un vector, w= k * u, renvoie w.
    Vector operator*( const Vector& v ) const
    {
        return Vector( v.x*x, v.y*y, v.z*z );
    }
    
    //! produit par un reel, w= k * u, renvoie w.
    Vector operator*( const float f ) const
    {
        return Vector( f*x, f*y, f*z );
    }
    
    //! produit par un reel, u= k * u.
    Vector &operator*=( const float f )
    {
        x *= f;
        y *= f;
        z *= f;
        
        return *this;
    }
    
    //! division par un reel, w= u / k, renvoie w.
    Vector operator/( const float f ) const
    {
        assert( f != 0 );
        float inv = 1.f / f;
        return Vector( x * inv, y * inv, z * inv );
    }
    
    //! division par un reel, u= u / k.
    Vector &operator/=( const float f )
    {
        assert( f != 0 );
        float inv = 1.f / f;
        x *= inv;
        y *= inv;
        z *= inv;
        
        return *this;
    }
    
    //! negation d'un vecteur, w= -u, renvoie w.
    Vector operator-( ) const
    {
        return Vector( -x, -y, -z );
    }
    
    //! renvoie une composante du vecteur.
    const float& operator[]( const unsigned int i ) const
    {
        return ( &x )[i];
    }
    
    //! renvoie une reference sur une composante du vecteur.
    float &operator[]( const unsigned int i )
    {
        return ( &x )[i];
    }
    
    //! renvoie le carre de la longueur du vecteur.
    float LengthSquared() const 
    { 
        return x*x + y*y + z*z; 
    }
    
    //! renvoie la longueur du vecteur.
    float Length() const 
    {
        return sqrtf( LengthSquared() ); 
    }
    
    // Vector Public Data
    //! composantes du vecteur.
    float x, y, z;
};

float Clamp( const float value , const float low, const float high );


//! representation d'un point de dimension 3. memes operations que sur Vector.
//! memes operations que sur un Vector. 
class Point
{
public:
    // Point Methods
    Point( )
        :
        x(0.f), y(0.f), z(0.f)
    {}
    
    Point( const float v )
        :
        x(v), y(v), z(v)
    {}
    
    Point( const float _x, const float _y, const float _z )
        :
        x( _x ), y( _y ), z( _z )
    {}
    
    explicit Point( const Vector &v )
        : 
        x( v.x ), y( v.y ), z( v.z ) 
    {}
    
    //! affiche un point.
    void print( ) const
    {
        printf("%.10f %.10f %.10f\n", x, y, z);
    }
    
    //! addition d'un point et d'un vecteur, q= p + v, renvoie le point q.
    Point operator+( const Vector &v ) const
    {
        return Point( x + v.x, y + v.y, z + v.z );
    }
    
    //! addition d'un point et d'un vecteur, p= p + v.
    Point &operator+=( const Vector &v )
    {
        x += v.x;
        y += v.y;
        z += v.z;
        
        return *this;
    }
    
    //! soustraction de 2 points, v= p - q, renvoie le vecteur v.
    Vector operator-( const Point &q ) const
    {
        return Vector( x - q.x, y - q.y, z - q.z );
    }
    
    //! soustraction d'un point et d'un vecteur, q= p - v, renvoie le point q.
    Point operator-( const Vector &v ) const
    {
        return Point( x - v.x, y - v.y, z - v.z );
    }
    
    //! soutraction d'un point et d'un vecteur, p= p - v.
    Point &operator-=( const Vector &v )
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        
        return *this;
    }
    
    //! addition de 2 points, ca n'existe pas, mais c'est pratique ! p= p + q.
    Point &operator+=( const Point &q )
    {
        x += q.x;
        y += q.y;
        z += q.z;
        
        return *this;
    }
    
    Point operator+( const Point &q ) const
    {
        return Point( x + q.x, y + q.y, z + q.z );
    }
    
    Point operator*( const float f ) const
    {
        return Point( f*x, f*y, f*z );
    }
    
    Point &operator*=( const float f )
    {
        x *= f;
        y *= f;
        z *= f;
        
        return *this;
    }
    
    Point operator/ ( const float f ) const
    {
        float inv = 1.f / f;
        return Point( inv*x, inv*y, inv*z );
    }
    
    Point &operator/=( const float f )
    {
        float inv = 1.f / f;
        x *= inv;
        y *= inv;
        z *= inv;
        
        return *this;
    }
    
    bool operator== ( const Point& p ) const
    {
        return (x == p.x && y == p.y && z == p.z);
    }
    
    const float& operator[]( const unsigned int i ) const
    {
        return ( &x )[i];
    }
    
    float &operator[]( const unsigned int i )
    {
        return ( &x )[i];
    }
    
    // Point Public Data
    float x, y, z;
};


//! representation d'une normale de dimension 3, cf. Vector pour la description des operations.
class Normal
{
public:
    // Normal Methods
    Normal( const float _x = 0.f, const float _y = 0.f, const float _z = 0.f )
        : 
        x( _x ), y( _y ), z( _z ) 
    {}
    
    Normal operator-( ) const
    {
        return Normal( -x, -y, -z );
    }
    
    Normal operator+ ( const Normal &v ) const
    {
        return Normal( x + v.x, y + v.y, z + v.z );
    }
    
    Normal& operator+=( const Normal &v )
    {
        x += v.x;
        y += v.y;
        z += v.z;
        
        return *this;
    }
    
    Normal operator- ( const Normal &v ) const
    {
        return Normal( x - v.x, y - v.y, z - v.z );
    }
    
    Normal& operator-=( const Normal &v )
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        
        return *this;
    }
    
    Normal operator*( const float f ) const
    {
        return Normal( f*x, f*y, f*z );
    }
    
    Normal &operator*=( const float f )
    {
        x *= f;
        y *= f;
        z *= f;
        
        return *this;
    }
    
    Normal operator/ ( const float f ) const
    {
        float inv = 1.f / f;
        return Normal( x * inv, y * inv, z * inv );
    }
    
    Normal &operator/=( float f )
    {
        float inv = 1.f / f;
        x *= inv;
        y *= inv;
        z *= inv;
        
        return *this;
    }
    
    float LengthSquared() const
    {
        return x*x + y*y + z*z;
    }
    
    float Length() const
    {
        return sqrtf( LengthSquared() );
    }
    
    explicit Normal( const Vector &v )
        : 
        x( v.x ), y( v.y ), z( v.z ) 
    {}
    
    const float& operator[]( const unsigned int i ) const 
    {
        return ( &x )[i];
    }
    
    float &operator[]( const unsigned int i )
    {
        return ( &x )[i]; 
    }
    
    // Normal Public Data
    float x, y, z;
};


// Geometry Inline Functions
inline 
Vector::Vector( const Point &p )
    : 
    x( p.x ), y( p.y ), z( p.z )
{}

inline
Vector::Vector( const Point& p, const Point& q )
    :
    x( q.x - p.x ), y( q.y - p.y ), z( q.z - p.z )
{}

//! scalaire * vecteur.
inline 
Vector operator*( float f, const Vector &v )
{
    return v*f;
}

//! produit scalaire de 2 vecteurs.
 inline 
float Dot( const Vector &v1, const Vector &v2 )
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

//! valeur absolue du produit scalaire de 2 vecteurs.
 inline 
float AbsDot( const Vector &v1, const Vector &v2 )
{
    return fabsf( Dot( v1, v2 ) );
}

//! max(0, dot) du produit scalaire de 2 vecteurs.
 inline 
float ZeroDot( const Vector &v1, const Vector &v2 )
{
    return std::max( 0.f, Dot( v1, v2 ) );
}

//! produit vectoriel de 2 vecteurs.
 inline 
Vector Cross( const Vector &v1, const Vector &v2 )
{
    return Vector(
        ( v1.y * v2.z ) - ( v1.z * v2.y ),
        ( v1.z * v2.x ) - ( v1.x * v2.z ),
        ( v1.x * v2.y ) - ( v1.y * v2.x ) );
}

//! produit vectoriel d'un vecteur et d'une normale.
 inline 
Vector Cross( const Vector &v1, const Normal &v2 )
{
    return Vector(
        ( v1.y * v2.z ) - ( v1.z * v2.y ),
        ( v1.z * v2.x ) - ( v1.x * v2.z ),
        ( v1.x * v2.y ) - ( v1.y * v2.x ) );
}

//! produit vectoriel d'une normale et d'un vecteur.
 inline 
Vector Cross( const Normal &v1, const Vector &v2 )
{
    return Vector(
        ( v1.y * v2.z ) - ( v1.z * v2.y ),
        ( v1.z * v2.x ) - ( v1.x * v2.z ),
        ( v1.x * v2.y ) - ( v1.y * v2.x ) );
}

//! renvoie un vecteur de longueur 1 de meme direction que v.
inline 
Vector Normalize( const Vector &v )
{
    return v / v.Length();
}

//! construit un repere orthogonal dont la normale est aligne sur un vecteur v1, v2 et v3 sont les 2 tangentes.
inline 
void CoordinateSystem( const Vector &v1, Vector *v2, Vector *v3 )
{
    if ( fabsf( v1.x ) > fabsf( v1.y ) )
    {
        float invLen = 1.f / sqrtf( v1.x * v1.x + v1.z * v1.z );
        *v2 = Vector( -v1.z * invLen, 0.f, v1.x * invLen );
    }
    else
    {
        float invLen = 1.f / sqrtf( v1.y * v1.y + v1.z * v1.z );
        *v2 = Vector( 0.f, v1.z * invLen, -v1.y * invLen );
    }
    
    *v3 = Cross( v1, *v2 );
    //~ *v3 = Cross( *v2, v1 );
}

//! renvoie la distance entre 2 points.
inline 
float Distance( const Point &p1, const Point &p2 )
{
    return ( p1 - p2 ).Length();
}

//! renvoie le carre de la distance entre 2 points.
inline 
float DistanceSquared( const Point &p1, const Point &p2 )
{
    return ( p1 - p2 ).LengthSquared();
}

//! scalaire * point.
inline 
Point operator*( float f, const Point &p )
{
    return p*f;
}

//! scalaire * normale.
inline 
Normal operator*( float f, const Normal &n )
{
    return Normal( f*n.x, f*n.y, f*n.z );
}

//! renvoie une normale de meme direction, mais de longeur 1.
 inline 
Normal Normalize( const Normal &n )
{
    return n / n.Length();
}

inline 
Vector::Vector( const Normal &n )
    :
    x( n.x ), y( n.y ), z( n.z )
{}

//! produit scalaire d'une normale et d'un vecteur.
 inline 
float Dot( const Normal &n1, const Vector &v2 )
{
    return n1.x * v2.x + n1.y * v2.y + n1.z * v2.z;
}

//! produit scalaire d'un vecteur et d'une normale.
 inline 
float Dot( const Vector &v1, const Normal &n2 )
{
    return v1.x * n2.x + v1.y * n2.y + v1.z * n2.z;
}

//! produit scalaire de 2 normales.
 inline 
float Dot( const Normal &n1, const Normal &n2 )
{
    return n1.x * n2.x + n1.y * n2.y + n1.z * n2.z;
}

//! valeur absolue du produit scalaire d'une normale et d'un vecteur.
 inline 
float AbsDot( const Normal &n1, const Vector &v2 )
{
    return fabsf( n1.x * v2.x + n1.y * v2.y + n1.z * v2.z );
}

//! valeur absolue du produit scalaire d'un vecteur et d'une normale.
 inline 
float AbsDot( const Vector &v1, const Normal &n2 )
{
    return fabsf( v1.x * n2.x + v1.y * n2.y + v1.z * n2.z );
}

//! valeur absolue du produit scalaire de 2 normales.
 inline 
float AbsDot( const Normal &n1, const Normal &n2 )
{
    return fabsf( n1.x * n2.x + n1.y * n2.y + n1.z * n2.z );
}

//! max(0, dot) du produit scalaire de 2 vecteurs.
 inline 
float ZeroDot( const Normal &v1, const Vector &v2 )
{
    return std::max( 0.f, Dot( v1, v2 ) );
}

//! max(0, dot) du produit scalaire de 2 vecteurs.
 inline 
float ZeroDot( const Vector &v1, const Normal &v2 )
{
    return std::max( 0.f, Dot( v1, v2 ) );
}

//! max(0, dot) du produit scalaire de 2 vecteurs.
 inline 
float ZeroDot( const Normal &v1, const Normal &v2 )
{
    return std::max( 0.f, Dot( v1, v2 ) );
}

//! interpolation lineaire entre 2 reels, x= (1 - t) v1 + t v2 
 inline 
float Lerp( const float t, const float v1, const float v2 ) 
{
    return (1.f - t) * v1 + t * v2;
}

//! limite une valeur entre un min et un max.
inline 
float Clamp( const float value, const float low, const float high)
{
    if(value < low) 
        return low;
    else if (value > high)
        return high;
    else 
        return value;
}

//! limite une valeur entre un min et un max.
inline 
int Clamp( const int value , const int low, const int high )
{
    if(value < low) 
        return low;
    else if (value > high) 
        return high;
    else 
        return value;
}

//! conversion degres vers radians.
 inline 
float Radians( const float deg )
{
	return ( (float) M_PI / 180.f) * deg;
}

//! conversion radians vers degres.
 inline 
float Degrees( const float rad )
{
    return (180.f / (float) M_PI) * rad;
}

//! renvoie un vecteur dont la direction est representee en coordonness polaires.
 inline 
Vector SphericalDirection( float sintheta, float costheta, float phi )
{
    return Vector( 
        sintheta * cosf( phi ), 
        sintheta * sinf( phi ), 
        costheta );
}

//! renvoie les composantes d'un vecteur dont la direction est representee en coordonnees polaires, dans la base x, y, z.
 inline 
Vector SphericalDirection( float sintheta, float costheta, float phi,
    const Vector &x, const Vector &y, const Vector &z )
{
    return sintheta * cosf( phi ) * x 
        + sintheta * sinf( phi ) * y 
        + costheta * z;
}

//! renvoie l'angle theta d'un vecteur avec la normale, l'axe Z, (utilisation dans un repere local).
 inline 
float SphericalTheta( const Vector &v )
{
    return acosf( Clamp( v.z, -1.f, 1.f ) );
}

//! renvoie l'angle phi d'un vecteur avec un vecteur tangent, l'axe X, (utilisation dans un repere local).
 inline 
float SphericalPhi( const Vector &v )
{
    float p = atan2f( v.y, v.x );
    return ( p < 0.f ) ? p + 2.f*M_PI : p;
}

} // namespace

#endif // PBRT_GEOMETRY_H
