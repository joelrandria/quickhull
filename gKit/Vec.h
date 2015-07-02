
#ifndef _GK_VEC_H
#define _GK_VEC_H

#include <cassert>
#include <cstdlib>


namespace gk {

//! classe de base pour representer un vecteur 2d.
template < typename T >
struct TVec2
{
    //! constructeur.
    TVec2( ) :  x( T(0) ), y( T(0) ) {}
    TVec2( const TVec2& v ) : x(v.x), y(v.y) {}
    
    explicit TVec2( const T _x, const T _y = T(0) ) :  x( _x ), y( _y ) {}
    
    //! construit un vecteur a partir de 2 Ts en memoire.
    explicit TVec2( const T * p ) { assert(p != NULL); x= p[0]; y= p[1]; }
    
    //! comparaison de 2 vecteurs.
    bool operator==( const TVec2 &v ) const { return (x == v.x && y == v.y); }
    //! renvoie une composante du vecteur.
    const T& operator[]( const unsigned int i ) const { return (&x)[i]; }
    //! renvoie une reference sur une composante du vecteur.
    T &operator[]( const unsigned int i ) { return (&x)[i]; }
    
    //! composantes du vecteur.
    union { T x; T r; };
    union { T y; T g; };
};

typedef TVec2<float> Vec2;

//! classe de base pour representer un vecteur / point, etc. 3d.
template < typename T >
struct TVec3
{
    //! constructeur.
    TVec3( ) :  x( T(0) ), y( T(0) ), z( T(0) ) {}
    TVec3( const TVec3& v ) : x(v.x), y(v.y), z(v.z) {}
    
    explicit TVec3( const T _x, const T _y = T(0), const T _z = T(0) ) :  x( _x ), y( _y ), z( _z ) {}
    explicit TVec3( const TVec2<T>& _v, const T _z = T(0) ) :  x( _v.x ), y( _v.y ), z( _z ) {}
    
    //! construit un vecteur a partir de 3 Ts en memoire.
    explicit TVec3( const T * p ) { assert(p != NULL); x= p[0]; y= p[1]; z= p[2]; }
    
    //! comparaison de 2 vecteurs.
    bool operator==( const TVec3 &v ) const { return (x == v.x && y == v.y && z == v.z); }
    //! renvoie une composante du vecteur.
    const T& operator[]( const unsigned int i ) const { return (&x)[i]; }
    //! renvoie une reference sur une composante du vecteur.
    T &operator[]( const unsigned int i ) { return (&x)[i]; }
    
    //! composantes du vecteur.
    union { T x; T r; T s; };
    union { T y; T g; T t; };
    union { T z; T b; T p; };
};

typedef TVec3<float> Vec3;


//! classe de base pour representer un point homogene, un vecteur homogene, une couleur, etc. 4d.
template < typename T >
struct TVec4
{
    //! constructeur.
    TVec4( ) :  x( T(0) ), y( T(0) ), z( T(0) ), w( T(0) ) {}
    TVec4( const TVec4& v ) : x(v.x), y(v.y), z(v.z), w(v.w) {}
    
    explicit TVec4( const T _x, const T _y = T(0), const T _z = T(0), const T _w= T(0) ) :  x( _x ), y( _y ), z( _z ), w( _w )  {}
    explicit TVec4( const TVec2<T>& _v, const T _z = T(0), const T _w= T(0) ) :  x( _v.x ), y( _v.y ), z( _z ), w( _w )  {}
    explicit TVec4( const TVec3<T>& _v, const T _w= T(0) ) :  x( _v.x ), y( _v.y ), z( _v.z ), w( _w )  {}
    
    //! construit un vecteur a partir de 4 Ts en memoire.
    explicit TVec4( const T * p ) { assert(p != NULL); x= p[0]; y= p[1]; z= p[2]; w= p[3]; }
    
    //! comparaison de 2 vecteurs.
    bool operator==( const TVec4 &v ) const { return (x == v.x && y == v.y && z == v.z && w == v.w); }
    //! renvoie une composante du vecteur.
    const T& operator[]( const unsigned int i ) const { return (&x)[i]; }
    //! renvoie une reference sur une composante du vecteur.
    T &operator[]( const unsigned int i ) { return (&x)[i]; }
    
    //! composantes du vecteur.
    union { T x; T r; T s; };
    union { T y; T g; T t; };
    union { T z; T b; T p; };
    union { T w; T a; T q; };
};

typedef TVec4<float> Vec4;


//! classe de base pour representer une matrice de transformation.
struct Mat4
{
    Mat4( )
    {
        for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            m[i][j] = 0.f;
        
        for(int k= 0; k < 4; k++)
            m[k][k]= 1.f;
    }
    
    //! construit une matrice a partir d'un tableau 2d de reels [ligne][colonne].
    Mat4( const float mat[4][4] ) 
    {
        for(int i = 0; i < 4; ++i)
        for(int j = 0; j < 4; ++j)
            m[i][j] = mat[i][j];
    }
    
    //! construit une matrice a partir des 16 elements.
    Mat4( 
       float t00, float t01, float t02, float t03,
       float t10, float t11, float t12, float t13,
       float t20, float t21, float t22, float t23,
       float t30, float t31, float t32, float t33 )
    {
        m[0][0] = t00; m[0][1] = t01; m[0][2] = t02; m[0][3] = t03;
        m[1][0] = t10; m[1][1] = t11; m[1][2] = t12; m[1][3] = t13;
        m[2][0] = t20; m[2][1] = t21; m[2][2] = t22; m[2][3] = t23;
        m[3][0] = t30; m[3][1] = t31; m[3][2] = t32; m[3][3] = t33;
    }
    
    //! conversion en float (*)[4]
    operator float *( ) { return (float *) m; }
    
    //! conversion en const float (*)[4]
    operator const float *( ) const { return (const float *) m; }

    float m[4][4];
};

}       // namespace

#endif
