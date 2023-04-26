// Copyright (c) 2017-2023, Lawrence Livermore National Security, LLC and
// other Tribol Project Developers. See the top-level LICENSE file for details.
//
// SPDX-License-Identifier: (MIT)

// Tribol includes
#include "tribol/types.hpp"
#include "tribol/mesh/MeshData.hpp"
#include "tribol/mesh/MethodCouplingData.hpp"
#include "tribol/integ/Integration.hpp"
#include "tribol/geom/GeomUtilities.hpp"
#include "tribol/integ/FE.hpp"

// Axom includes
#include "axom/slic.hpp"

// gtest includes
#include "gtest/gtest.h"

// c++ includes
#include <cmath> // std::abs

using real = tribol::real;

/*!
 * Test fixture class; some setup is necessary to test the evaluation of 
 * integrals of quad 4 shape functions to compute the area of a quadrilateral 
 * using Gauss quadrature and an isoparametric four node quad.
 */
class IsoIntegTest : public ::testing::Test
{
   
public:
   int numNodes;
   int dim;

   real* getXCoords() 
   {
      return x;
   }

   real* getYCoords() 
   {
      return y;
   }

   real* getZCoords() 
   {
      return z;
   }

   bool integrate ( real const tol )
   {
      real xyz[ this->dim * this->numNodes ];
      real* xy = xyz;

      real * x = this->x;
      real * y = this->y;
      real * z = this->z;

      // generate stacked coordinate array
      for (int j=0; j<this->numNodes; ++j)
      {
         for (int k=0; k<this->dim; ++k)
         {
            switch (k)
            {
               case 0:
                  xy[ this->dim * j + k ] = x[ j ];
                  break;
               case 1:
                  xy[ this->dim * j + k ] = y[ j ];
                  break;
               case 2:
                  xy[ this->dim * j + k ] = z[ j ];
                  break;
            } // end switch
         } // end loop over dimension
      } // end loop over nodes

      // instantiate SurfaceContactElem struct. Note, this object is instantiated 
      // using face 1 as face 2, but these faces are not used in this test so this 
      // is ok.
      tribol::SurfaceContactElem elem ( this->dim, xy, xy, xy, 
                                        this->numNodes, this->numNodes, 
                                        0, 1, 0, 0);

      // instantiate integration object
      tribol::IntegPts integ;

      // generate all current configuration integration point coordinates and weights
      tribol::GaussPolyIntQuad( elem, integ, 2 );

      // evaluate sum_a (integral_face (phi_a) da) with outer loop over nodes, a, and 
      // inner loop over number of integration points
      real areaTest = 0.;
      real phi = 0.;

      for (int a=0; a<this->numNodes; ++a)
      {
         for (int ip=0; ip<integ.numIPs; ++ip)
         {
            real xi[2];

            // access (xi,eta) coordinates. Note that the integration point coordinates 
            // for this method are not using a zeta=0 component. That is, the stride is 2, 
            // not 3.
            xi[0] = integ.xy[integ.ipDim*ip];
            xi[1] = integ.xy[integ.ipDim*ip+1];

            tribol::LinIsoQuadShapeFunc( xi[0], xi[1], a, phi );
       
            // compute determinant of the Jacobian of the transformation
            real dJ;
            tribol::DetJQuad( xi[0], xi[1], elem.overlapCoords, elem.dim, dJ );

            areaTest += integ.wts[ip]*phi*dJ;

         }
      }

      real area = tribol::Area2DPolygon( x, y, this->numNodes );

      bool convrg = (std::abs(areaTest - area) <= tol) ? true : false;
  
      return convrg;

   }

protected:

   void SetUp() override
   {
      this->numNodes = 4;
      this->dim = 3;

      if (this->x == nullptr)
      {
         this->x = new real [this->numNodes];
      }
      else
      {
         delete [] this->x;
         this->x = new real [this->numNodes];
      }

      if (this->y == nullptr)
      {
         this->y = new real [this->numNodes];
      }
      else
      {
         delete [] this->y;
         this->y = new real [this->numNodes];
      }

      if (this->z == nullptr)
      {
         this->z = new real [this->numNodes];
      }
      else
      {
         delete [] this->z;
         this->z = new real [this->numNodes];
      }
   }

   void TearDown() override
   {
      if (this->x != nullptr)
      {
         delete [] this->x;
         this->x = nullptr;
      }
      if (this->y != nullptr)
      {
         delete [] this->y;
         this->y = nullptr;
      }
      if (this->z != nullptr)
      {
         delete [] this->z;
         this->z = nullptr;
      }
   }

protected:

   real* x {nullptr};
   real* y {nullptr};
   real* z {nullptr};

};


TEST_F( IsoIntegTest, square )
{

   real* x = this->getXCoords();
   real* y = this->getYCoords();
   real* z = this->getZCoords();

   x[0] = -0.5;
   x[1] =  0.5;
   x[2] =  0.5;
   x[3] = -0.5;

   y[0] = -0.5;
   y[1] = -0.5;
   y[2] = 0.5;
   y[3] = 0.5;

   z[0] = 0.1;
   z[1] = 0.1;
   z[2] = 0.1; 
   z[3] = 0.1;

   bool convrg = this->integrate( 1.e-8 );

   EXPECT_EQ( convrg, true );
}

TEST_F( IsoIntegTest, rect )
{

   real* x = this->getXCoords();
   real* y = this->getYCoords();
   real* z = this->getZCoords();

   x[0] = -0.5;
   x[1] =  0.5;
   x[2] =  0.5;
   x[3] = -0.5;

   y[0] = -0.25;
   y[1] = -0.25;
   y[2] = 0.25;
   y[3] = 0.25;

   z[0] = 0.1;
   z[1] = 0.1;
   z[2] = 0.1; 
   z[3] = 0.1;

   bool convrg = this->integrate( 1.e-8 );

   EXPECT_EQ( convrg, true );
}

TEST_F( IsoIntegTest, affine )
{
   real* x = this->getXCoords();
   real* y = this->getYCoords();
   real* z = this->getZCoords();

   x[0] = -0.5;
   x[1] =  0.5;
   x[2] =  0.8;
   x[3] = -0.2;

   y[0] = -0.415;
   y[1] = -0.415;
   y[2] = 0.5;
   y[3] = 0.5;

   z[0] = 0.1;
   z[1] = 0.1;
   z[2] = 0.1; 
   z[3] = 0.1;

   bool convrg = integrate( 1.e-5 );

   EXPECT_EQ( convrg, true );
}

TEST_F( IsoIntegTest, nonaffine )
{
   real* x = this->getXCoords();
   real* y = this->getYCoords();
   real* z = this->getZCoords();

   x[0] = -0.5;
   x[1] =  0.5;
   x[2] =  0.235;
   x[3] = -0.35;

   y[0] = -0.25;
   y[1] = -0.15;
   y[2] = 0.25;
   y[3] = 0.235;

   z[0] = 0.1;
   z[1] = 0.1;
   z[2] = 0.1; 
   z[3] = 0.1;

   bool convrg = integrate( 1.e-8 );

   EXPECT_EQ( convrg, true );
}

int main(int argc, char* argv[])
{
  int result = 0;

  ::testing::InitGoogleTest(&argc, argv);

  axom::slic::SimpleLogger logger;

  result = RUN_ALL_TESTS();

  return result;
}
