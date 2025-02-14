


#include <algorithm>

#include <cmath>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <string>

#include "LRL_Cell.h"
#include "C3.h"
#include "B4.h"
#include "D7.h"
#include "Delone.h"
#include "G6.h"
#include "LRL_MinMaxTools.h"
#include "LRL_RandTools.h"
#include "MatG6.h"
#include "MatS6.h"
#include "Mat66.h"
#include "rhrand.h"
#include "S6.h"
#include "Selling.h"
#include "LRL_StringTools.h"
#include "S6M_SellingReduce.h"

int randSeed1 = 19191;

const double thirtyDegrees = 30.0 / 180.0 * 4 * atan(1.0);
const double sixtyDegrees = 2.0*thirtyDegrees;
const double ninetyDegrees = 3.0*thirtyDegrees;
const double oneeightyDegrees = 180.0;
const double threesixtyDegrees = 360.0;

double LRL_Cell::randomLatticeNormalizationConstant = 10.0;
double LRL_Cell::randomLatticeNormalizationConstantSquared = randomLatticeNormalizationConstant * randomLatticeNormalizationConstant;

/*  class LRL_Cell
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
A class to implement some common operations for unit cells as usually 
   used with xray crystallography. Conversions from G6 to unit cell and 
   from unit to G6 are included. The angles are ALWAYS in RADIANS.

   LRL_Cell(void)                                  == default constructor
   LRL_Cell( const G6& v )                 == constructor to convert a G6 vector to unit cell
   double Volume(void)                         == return the volume of a unit cell
   double LRL_Cell::operator[](const int& n) const == return the n-th element of a cell (zero-based)
   LRL_Cell LRL_Cell::Inverse( void ) const            == compute the reciprocal cell
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// Name: LRL_Cell()
// Description: default constructor
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
LRL_Cell::LRL_Cell(void)
   : m_valid(false)
{
   m_cell.resize(6);
}

LRL_Cell::LRL_Cell(const LRL_Cell& c)
   : m_cell(c.m_cell)
   , m_valid(c.m_valid)
{
}

LRL_Cell::LRL_Cell(const std::string& s)
   : m_valid(true)
{
   m_cell = LRL_StringTools::FromString(s);
   m_valid = m_valid && m_cell[3] < oneeightyDegrees && m_cell[4] < oneeightyDegrees && m_cell[5] < oneeightyDegrees && (m_cell[3]+m_cell[4]+m_cell[5])< threesixtyDegrees;

   for (size_t i = 3; i < 6; ++i)
      m_cell[i] *= 4.0*atan(1.0) / 180.0;
}

LRL_Cell::LRL_Cell(const S6& ds)
{
   static const double pi = 4.0*atan(1.0);
   static const double twopi = 2.0*pi;
   m_valid = true;
   const G6 g6(ds);
   *this = g6;
   m_valid = m_valid && ds.GetValid() && GetValid() && m_cell[3] < pi && m_cell[4] < pi && m_cell[5] < pi && (m_cell[3] + m_cell[4] + m_cell[5])< twopi
      && (m_cell[3] + m_cell[4] + m_cell[5] - 2.0 * maxNC(m_cell[3],m_cell[4],m_cell[5]) >= 0.0);
} 

LRL_Cell::LRL_Cell(const C3& c3)
{
   static const double pi = 4.0*atan(1.0);
   static const double twopi = 2.0*pi;
   *this = S6(c3);
   m_valid = m_valid && c3.GetValid() && GetValid() && m_cell[3] < pi && m_cell[4] < pi && m_cell[5] < pi && (m_cell[3] + m_cell[4] + m_cell[5])< twopi
      && (m_cell[3] + m_cell[4] + m_cell[5] - 2.0 * maxNC(m_cell[3], m_cell[4], m_cell[5]) >= 0.0);
}

LRL_Cell::LRL_Cell(const B4& dt)
{
   (*this) = G6(dt);
}

std::ostream& operator<< (std::ostream& o, const LRL_Cell& c) {
   std::streamsize oldPrecision = o.precision();
   o << std::fixed << std::setprecision(5);
   for (size_t i = 0; i < 6; ++i)
      o << std::setw(9) << c.m_cell[i] << " ";
   o << std::setprecision(oldPrecision);
   o.unsetf(std::ios::floatfield);
   return o;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// Name: LRL_Cell()
// Description: constructor to convert load a LRL_Cell from doubles
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
LRL_Cell::LRL_Cell( const double a, const double b, const double c,
           const double alpha, const double beta, const double gamma)
   : m_valid(true)
{
   m_cell.resize( 6 );
   m_cell[0] = a;
   m_cell[1] = b;
   m_cell[2] = c;
   m_cell[3] = alpha / 57.2957795130823;
   m_cell[4] = beta / 57.2957795130823;
   m_cell[5] = gamma / 57.2957795130823;
   const double lowerlimit = 0.001;
   m_valid = m_valid && a > lowerlimit && b > lowerlimit && c > lowerlimit && alpha > lowerlimit && beta > lowerlimit && gamma > lowerlimit
      && alpha < 179.99 && beta < 179.99 && gamma < 179.99 && (alpha + beta + gamma) < 360.0 &&
      (alpha + beta + gamma - 2.0*maxNC(alpha, beta, gamma) >= 0.0);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// Name: LRL_Cell()
// Description: constructor to convert an input G6 vector (as a vector of
//              doubles to E3 lengths and angles. The angles are 
//              stored as RADIANS (only). For angles as degrees, use the
//              class LRL_Cell_Degrees.
//              with lengths and angles as DEGREES. For consistency, a
//              "LRL_Cell" object will never contain degrees.
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
LRL_Cell::LRL_Cell(const G6& g6)
   : m_valid(true)
{
   static const double pi = 4.0*atan(1.0);
   static const double twopi = 2.0*pi;
   m_cell.resize(6);
   const double lowerlimit = 0.0001;
   if ( (!g6.GetValid()) || g6.norm() < 1.0E-10 || g6[0] <= lowerlimit || g6[1] <= lowerlimit || g6[2] <= lowerlimit) {
      *this = LRL_Cell(0, 0, 0, 0, 0, 0);
      return;
   }
   else {
      m_cell[0] = sqrt(g6[0]);
      m_cell[1] = sqrt(g6[1]) ;
      m_cell[2] = sqrt(g6[2]);

      const double cosalpha(0.5*g6[3] / (m_cell[1] * m_cell[2]));
      const double cosbeta(0.5*g6[4] /  (m_cell[0] * m_cell[2]));
      const double cosgamma(0.5*g6[5] / (m_cell[0] * m_cell[1]));

      if (std::abs(cosalpha) >= 0.9999 || std::abs(cosbeta) >= 0.9999 || std::abs(cosgamma) >= 0.9999) {
         *this = LRL_Cell(0, 0, 0, 0, 0, 0);
         m_valid = false;
         return;
      }
      const double sinalpha(sqrt(1.0 - cosalpha * cosalpha));
      const double sinbeta(sqrt(1.0 - cosbeta * cosbeta));
      const double singamma(sqrt(1.0 - cosgamma * cosgamma));

      // compute the v angles in radians
      m_cell[3] = atan2(sinalpha, cosalpha);
      m_cell[4] = atan2(sinbeta, cosbeta);
      m_cell[5] = atan2(singamma, cosgamma);
      m_valid = m_valid && m_cell[0] > lowerlimit && m_cell[1] > lowerlimit && m_cell[2] > lowerlimit &&
         m_cell[3] > lowerlimit && m_cell[4] > lowerlimit && m_cell[5] > lowerlimit &&
         m_cell[3] < pi && m_cell[4] < pi && m_cell[5] < pi && (m_cell[3] + m_cell[4] + m_cell[5])< twopi
         && (m_cell[3] + m_cell[4] + m_cell[5] - 2.0 * maxNC(m_cell[3], m_cell[4], m_cell[5]) >= 0.0);
   }
}

LRL_Cell::LRL_Cell(const D7& v7)
   : m_valid(v7.GetValid())
{
   (*this) = G6(v7);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// Name: LRL_Cell()
// Description: destructor
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
LRL_Cell::~LRL_Cell(void)
{
}

void Prepare2LRL_CellElements( const double minEdge, const double maxEdge, const size_t i, LRL_Cell& c ) {
   static RHrand r;
   const double range = std::fabs( minEdge - maxEdge );
   const double d1 = r.urand( );
   c[i] = range * d1 + std::sqrt( minEdge * maxEdge );
   const double d2 = r.urand( );
   c[i + 3] = d2 * oneeightyDegrees;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// Name: Volume()
// Description: Return the E3 volume of a LRL_Cell
// follows the formula of Stout and Jensen page 33
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
double LRL_Cell::Volume( void ) const
{
   const G6& c( m_cell );
   const double c3(cos(c[3]));
   const double c4(cos(c[4]));
   const double c5(cos(c[5]));
   const double volume( c[0]*c[1]*c[2] * sqrt( 1.0-pow(c3,2)-pow(c4,2)-pow(c5,2)
       + 2.0*c3*c4*c5 ) );
   return volume;
}


bool LRL_Cell::operator== (const LRL_Cell& cl) const {
   return m_cell == cl.m_cell;
}
bool LRL_Cell::operator!= (const LRL_Cell& cl) const {
   return !((*this) == cl);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// Name: operator[]()
// Description: access function for the values in a LRL_Cell object
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
double LRL_Cell::operator[](const size_t n) const
{
   const size_t nn( std::max(0UL,std::min(5UL,n)) );
   return m_cell[nn];
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// Name: operator[]()
// Description: access function for the values in a LRL_Cell object
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
double& LRL_Cell::operator[](const size_t n)
{
   const size_t nn( std::max(0UL,std::min(5UL,n)) );
   return m_cell[nn];
}

double LRL_Cell::DistanceBetween(const LRL_Cell& v1, const LRL_Cell& v2) {
   return (B4(v1) - B4(v2)).norm();
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// Name: Inverse()
// Description: Compute the reciprocal cell (inverse) of a cell
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
LRL_Cell LRL_Cell::Inverse( void ) const
{
   const double& a((*this)[0]);
   const double& b((*this)[1]);
   const double& c((*this)[2]);
   const double& alpha((*this)[3]);
   const double& beta ((*this)[4]);
   const double& gamma((*this)[5]);
   const double cosAlpha(cos(alpha));
   const double cosBeta (cos(beta));
   const double cosGamma(cos(gamma));
   const double sinAlpha(sin(alpha));
   const double sinBeta (sin(beta));
   const double sinGamma(sin(gamma));

   const double v     = (*this).Volume( );
   const double astar = b*c*sin(alpha)/v;
   const double bstar = a*c*sin(beta )/v;
   const double cstar = a*b*sin(gamma)/v;

   const double cosAlphaStar = (cosBeta *cosGamma-cosAlpha)/fabs(sinBeta*sinGamma);
   const double cosBetaStar  = (cosAlpha*cosGamma-cosBeta )/fabs(sinAlpha*sinGamma);
   const double cosGammaStar = (cosAlpha*cosBeta -cosGamma)/fabs(sinAlpha*sinBeta);

   LRL_Cell cell;
   cell.m_cell[0] = astar;
   cell.m_cell[1] = bstar;
   cell.m_cell[2] = cstar;
   cell.m_cell[3] = atan2( sqrt(1.0-pow(cosAlphaStar,2)), cosAlphaStar);
   cell.m_cell[4] = atan2( sqrt(1.0-pow(cosBetaStar ,2)), cosBetaStar );
   cell.m_cell[5] = atan2( sqrt(1.0-pow(cosGammaStar,2)), cosGammaStar);
   cell.m_valid = m_valid;

   return cell;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// Name: Cell2V6()
// Description: return the G6 vector of an input cell
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
G6 LRL_Cell::Cell2V6( void ) const
{
   const LRL_Cell& c( *this );
   G6 v;
   v[0] = c[0]*c[0];
   v[1] = c[1]*c[1];
   v[2] = c[2]*c[2];
   v[3] = 2.0*c[1]*c[2]*cos(c[3]);
   v[4] = 2.0*c[0]*c[2]*cos(c[4]);
   v[5] = 2.0*c[0]*c[1]*cos(c[5]);
   for ( size_t i=3; i<6; ++i ) if ( std::fabs(v[i]) < 1.0E-10 ) v[i] = 0.0;

   return v;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// Name: Cell2V6()
// Description: return the G6 vector of an input cell
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
G6 LRL_Cell::Cell2V6( const LRL_Cell& c )
{
   G6 v;
   v[0] = c[0]*c[0];
   v[1] = c[1]*c[1];
   v[2] = c[2]*c[2];
   v[3] = 2.0*c[1]*c[2]*cos(c[3]);
   v[4] = 2.0*c[0]*c[2]*cos(c[4]);
   v[5] = 2.0*c[0]*c[1]*cos(c[5]);
   
   return v;
}




/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// Name: LatSymMat66(const std::string& latsym)
// Description: return the Mat66 matrix for a lattice symbol
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
Mat66 LRL_Cell::LatSymMat66( const std::string& latsym )
{

   Mat66 mc;
   double dmc[36];
   G6 g6cent=(*this).Cell2V6();
   double primcell[6];
   char clatsym=latsym[0];
   CS6M_LatSymMat66(g6cent,clatsym,dmc,primcell);
   mc = Mat66(dmc);
   return mc;
}

Mat66 LRL_Cell::LatSymMat66( const std::string& latsym, const LRL_Cell& c ) {
   return (LRL_Cell(c)).LatSymMat66( latsym );
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// Name: LatSymMatG6(const std::string& latsym)
// Description: return the MatG6 matrix for a lattice symbol
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
MatG6 LRL_Cell::LatSymMatG6( const std::string& latsym )
{

   MatG6 mc;
   G6 g6cent=(*this).Cell2V6();
   G6 primcell;
   char clatsym=latsym[0];
   CS6M_LatSymMat66(g6cent,clatsym,mc,primcell);
   return mc;
}

MatG6 LRL_Cell::LatSymMatG6( const std::string& latsym, const LRL_Cell& c ) {
   return (LRL_Cell(c)).LatSymMatG6( latsym );
}

static int iseed;

LRL_Cell LRL_Cell::rand() {
   return S6::rand();
}

LRL_Cell LRL_Cell::randDeloneReduced() {//LRL_Cell::randomLatticeNormalizationConstant
   S6 vRan;
   S6 v;

   MatS6 m;
   vRan = LRL_Cell::rand();

   const bool test = Delone::Reduce(vRan, m, v);
   while (!Delone::IsReduced(vRan)) {
      vRan = LRL_Cell::rand();
   }
   return LRL_Cell(v);
}

LRL_Cell LRL_Cell::randDeloneUnreduced() {//LRL_Cell::randomLatticeNormalizationConstant
   S6 vRan;

   MatS6 m;
   vRan = LRL_Cell::rand();

   while (Delone::IsReduced(vRan)) {
      vRan = LRL_Cell::rand();
   }
   LRL_Cell celltemp(vRan);
   return LRL_Cell(vRan);
}

LRL_Cell LRL_Cell::rand(const double d) {//LRL_Cell::randomLatticeNormalizationConstant
   return d*rand()/ randomLatticeNormalizationConstant;
}

LRL_Cell LRL_Cell::randDeloneReduced(const double d) {//LRL_Cell::randomLatticeNormalizationConstant
   return d*randDeloneReduced()/ randomLatticeNormalizationConstant;
}

LRL_Cell LRL_Cell::randDeloneUnreduced(const double d) {//LRL_Cell::randomLatticeNormalizationConstant
   return d*randDeloneUnreduced()/ randomLatticeNormalizationConstant;
}



/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// Name: vvtorow(const double m,
//               const int n1, const int n2,
//               mat33& v1, mat33& v2,
//               const int n3,
//               MatG6& m6)
// Description: Compute one row of a 6x6 matrix from
//              2 rows of 2 3x3 matrices
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


LRL_Cell LRL_Cell::operator* (const double d) const {
   LRL_Cell c(*this);
   c[0] *= d;
   c[1] *= d;
   c[2] *= d;
   return c;
}

LRL_Cell LRL_Cell::operator/ (const double d) const {
   LRL_Cell c(*this);
   c[0] /= d;
   c[1] /= d;
   c[2] /= d;
   return c;
}

LRL_Cell LRL_Cell::operator+ (const LRL_Cell& c) const {
   const G6 v1(*this);
   const G6 v2(c);
   return LRL_Cell(G6(v1 + v2));
}

LRL_Cell LRL_Cell::operator- (const LRL_Cell& c) const {
   const G6 v1(*this);
   const G6 v2(c);
   const G6 diff = v1 - v2;
   if (diff.norm() < 1.0E-10)
      return LRL_Cell(0,0,0,0,0,0);
   else
      return LRL_Cell(G6(v1 - v2));
}

LRL_Cell& LRL_Cell::operator+= (const LRL_Cell& cl) {
   (*this) = (*this) + cl;
   return *this;
}

LRL_Cell& LRL_Cell::operator-= (const LRL_Cell& cl) {
   *this = (*this) - cl;
   return *this;
}

LRL_Cell operator* (const double d, const LRL_Cell& c) {
   return c*d;
}

LRL_Cell& LRL_Cell::operator= (const D7& v) {
   *this = LRL_Cell(v);
   return *this;
}

LRL_Cell& LRL_Cell::operator= (const C3& c3) {
   *this = LRL_Cell(c3);
   return *this;
}

LRL_Cell& LRL_Cell::operator= (const S6& v) {
   *this = LRL_Cell(v);
   return *this;
}

LRL_Cell& LRL_Cell::operator= (const B4& v) {
   *this = LRL_Cell(v);
   return *this;
}

LRL_Cell& LRL_Cell::operator= (const LRL_Cell& v) {
   m_cell = v.m_cell;
   m_valid = v.GetValid();
   return *this;
}

LRL_Cell& LRL_Cell::operator= (const G6& v) {
   *this = LRL_Cell(v);
   m_valid = m_valid && GetValid();
   return *this;
}

LRL_Cell& LRL_Cell::operator= (const std::string& s) {
   *this = LRL_Cell(s);
   m_valid = true;
   return *this;
}

LRL_Cell& LRL_Cell::operator/= (const double d) {
   (*this)[0] /= d;
   (*this)[1] /= d;
   (*this)[2] /= d;
   m_valid = true;
   return *this;
}

LRL_Cell& LRL_Cell::operator*= (const double d) {
   (*this)[0] *= d;
   (*this)[1] *= d;
   (*this)[2] *= d;
   m_valid = true;
   return *this;
}

LRL_Cell LRL_Cell::GetPrimitiveCell(const std::string& latsym, const LRL_Cell& c) {
   return LRL_Cell(GetPrimitiveV6Vector(latsym, c));
}

G6 LRL_Cell::GetPrimitiveV6Vector(const std::string& latsym, const LRL_Cell& c) {
   const MatG6 m66 = (LRL_Cell(c)).LatSymMatG6(latsym);
   return m66 * G6(c);
}
