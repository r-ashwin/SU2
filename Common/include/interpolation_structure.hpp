/*!
 * \file interpolation_structure.hpp
 * \brief Headers of the main subroutines used by SU2_FSI.
 *        The subroutines and functions are in the <i>interpolation_structure.cpp</i> file.
 * \author H. Kline
 * \version 4.0.0 "Cardinal"
 *
 * SU2 Lead Developers: Dr. Francisco Palacios (Francisco.D.Palacios@boeing.com).
 *                      Dr. Thomas D. Economon (economon@stanford.edu).
 *
 * SU2 Developers: Prof. Juan J. Alonso's group at Stanford University.
 *                 Prof. Piero Colonna's group at Delft University of Technology.
 *                 Prof. Nicolas R. Gauger's group at Kaiserslautern University of Technology.
 *                 Prof. Alberto Guardone's group at Polytechnic University of Milan.
 *                 Prof. Rafael Palacios' group at Imperial College London.
 *                 Prof. Edwin van der Weide's group at the University of Twente.
 *                 Prof. Vincent Terrapon's group at the University of Liege.
 *
 * Copyright (C) 2012-2016 SU2, the open-source CFD code.
 *
 * SU2 is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * SU2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with SU2. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "../../Common/include/mpi_structure.hpp"

#include <iostream>
#include <cmath>
#include <cstdlib>

#include "config_structure.hpp"
#include "geometry_structure.hpp"
#include "vector_structure.hpp"

using namespace std;


/*!
 * \class CInterpolator
 * \brief Main class for defining the interpolator, it requires
 * a child class for each particular interpolation method
 * \author H. Kline
 * \version 3.2.9 "eagle"
 */
class CInterpolator {
protected:
  unsigned int nZone;
  unsigned int donorZone, targetZone;

  unsigned long MaxLocalVertex_Donor,/*!\brief Maximum vertices per processor*/
  nGlobalFace_Donor,/*!\brief */
  nGlobalFaceNodes_Donor,/*!\brief */
  MaxFace_Donor,/*!\brief Maximum faces per processor*/
  MaxFaceNodes_Donor;/*!\brief Maximum nodes associated with faces per processor*/
  unsigned long *Buffer_Receive_nVertex_Donor, /*!\brief Buffer to store the number of vertices per processor on the Donor domain */
  *Buffer_Receive_nFace_Donor, /*!\brief Buffer to store the number of faces per processor*/
  *Buffer_Receive_nFaceNodes_Donor,/*!\brief Buffer to store the number of nodes associated with faces per processor*/
  *Buffer_Send_nVertex_Donor,/*!\brief Buffer to send number of vertices on the local processor*/
  *Buffer_Send_nFace_Donor,/*!\brief Buffer to send number of faces on the local processor*/
  *Buffer_Send_nFaceNodes_Donor,/*!\brief Buffer to send the number of nodes assocated with faces per processor*/
  *Buffer_Receive_GlobalPoint, /*!\brief Buffer to receive global point indices*/
  *Buffer_Send_GlobalPoint,/*!\brief Buffer to send global point indices*/
  *Buffer_Send_FaceIndex,/*!\brief Buffer to send indices pointing to the node indices that define the faces*/
  *Buffer_Receive_FaceIndex,/*!\brief Buffer to receive indices pointing to the node indices that define the faces*/
  *Buffer_Send_FaceNodes,/*!\brief Buffer to send indices pointing to the location of node information in other buffers, defining faces*/
  *Buffer_Receive_FaceNodes,/*!\brief Buffer to receive indices pointing to the location of node information in other buffers, defining faces*/
  *Buffer_Send_FaceProc,/*!\brief Buffer to send processor which stores the node indicated in Buffer_Receive_FaceNodes*/
  *Buffer_Receive_FaceProc;/*!\brief Buffer to receive processor which stores the node indicated in Buffer_Receive_FaceNodes*/

  su2double *Buffer_Send_Coord,/*!\brief Buffer to send coordinate values*/
  *Buffer_Send_Normal,/*!\brief Buffer to send normal vector values */
  *Buffer_Receive_Coord,/*!\brief Buffer to receive coordinate values*/
  *Buffer_Receive_Normal;/*!\brief Buffer to receive normal vector values*/



public:
  CGeometry*** Geometry; 		/*! \brief Vector which stores n zones of geometry. */
  CGeometry* donor_geometry; 	/*! \brief Vector which stores the donor geometry. */
  CGeometry* target_geometry; 	/*! \brief Vector which stores the target geometry. */

  /*!
   * \brief Constructor of the class.
   */
  CInterpolator(void);

  /*!
 * \brief Constructor of the class.
 * \param[in] geometry - Geometrical definition of the problem.
 * \param[in] config - Definition of the particular problem.
 * \param[in] iZone - index of the donor zone
 * \param[in] jZone - index of the target zone
 */
  CInterpolator(CGeometry ***geometry_container, CConfig **config, unsigned int iZone, unsigned int jZone);

  /*!
   * \brief Destructor of the class.
   */
  virtual ~CInterpolator(void);

  /*!
   * \brief Find the index of the interface marker shared by that zone
   * \param[in] config - Definition of the particular problem.
   * \param[in] val_marker_interface - Interface tag.
   */
  int Find_InterfaceMarker(CConfig *config, unsigned short val_marker_interface);

  /*!
   * \brief Set up transfer matrix defining relation between two meshes
   * \param[in] config - Definition of the particular problem.
   */
  virtual void Set_TransferCoeff(CConfig **config);

  /*!
   * \brief Determine array sizes used to collect and send coordinate and global point
   * information.
   * \param[in] faces - boolean that determines whether or not to set face information as well
   * \param[in] markDonor - Index of the boundary on the donor domain.
   * \param[in] markTarget - Index of the boundary on the target domain.
   * \param[in] nVertexDonor - Number of vertices on the donor boundary.
   * \param[in] nDim - number of physical dimensions.
   */
  void Determine_ArraySize(bool faces, int markDonor, int markTarget, unsigned long nVertexDonor, unsigned short nDim);

  /*!
   * \brief Collect and communicate vertex info: coord, global point, and if faces=true the normal vector
   * \param[in] faces - boolean that determines whether or not to set face information as well
   * \param[in] markDonor - Index of the boundary on the donor domain.
   * \param[in] markTarget - Index of the boundary on the target domain.
   * \param[in] nVertexDonor - Number of vertices on the donor boundary.
   * \param[in] nDim - number of physical dimensions.
   */
  void Collect_VertexInfo(bool faces, int markDonor, int markTarget, unsigned long nVertexDonor, unsigned short nDim);


};

/*!
 * \brief Nearest Neighbor interpolation
 */
class CNearestNeighbor : public CInterpolator {
public:

  /*!
   * \brief Constructor of the class.
   */
  CNearestNeighbor(void);

  /*!
   * \brief Constructor of the class.
   * \param[in] geometry - Geometrical definition of the problem.
   * \param[in] config - Definition of the particular problem.
   * \param[in] iZone - index of the donor zone
   * \param[in] jZone - index of the target zone
   */
  CNearestNeighbor(CGeometry ***geometry_container, CConfig **config, unsigned int iZone, unsigned int jZone);

  /*!
   * \brief Destructor of the class.
   */
  ~CNearestNeighbor(void);

  /*!
   * \brief Set up transfer matrix defining relation between two meshes
   * \param[in] config - Definition of the particular problem.
   */
  void Set_TransferCoeff(CConfig **config);

};

/*!
 * \brief Isoparametric interpolation
  */
class CIsoparametric : public CInterpolator {
public:

  /*!
   * \brief Constructor of the class.
   * \param[in] geometry - Geometrical definition of the problem.
   * \param[in] config - Definition of the particular problem.
   * \param[in] iZone - index of the donor zone
   * \param[in] jZone - index of the target zone
   */
  CIsoparametric(CGeometry ***geometry_container, CConfig **config, unsigned int iZone, unsigned int jZone);

  /*!
   * \brief Destructor of the class.
   */
  ~CIsoparametric(void);

  /*!
   * \brief Set up transfer matrix defining relation between two meshes
   * \param[in] config - Definition of the particular problem.
   */
  void Set_TransferCoeff(CConfig **config);

  /*!
   * \brief Calculate the isoparametric representation of point iVertex in marker iZone_0 by nodes of element donor_elem in marker jMarker of zone iZone_1.
   * \param[in] iVertex - vertex index of the point being interpolated.
   * \param[in] nDim - the dimension of the coordinates.
   * \param[in] iZone_1 - zone index of the element to use for interpolation (the DONOR zone)
   * \param[in] donor_elem - element index of the element to use for interpolation (or global index of a point in 2D)
   * \param[in[ nDonorPoints - number of donor points in the element.
   * \param[in] xj - point projected onto the plane of the donor element.
   * \param[out] isoparams - isoparametric coefficients. Must be allocated to size nNodes ahead of time. (size> nDonors)
   *
   * If the problem is 2D, the 'face' projected onto is actually an edge; the local index
   * of the edge is then stored in iFace, and the global index of the node (from which the edge
   * is referenced)
   */
  void Isoparameters(unsigned short nDim, unsigned short nDonor, su2double *X, su2double *xj,su2double* isoparams);

};

/*!
 * \brief Mirror interpolation: copy point linking and coefficient values from the opposing mesh
 * Assumes that the oppoosing mesh has already run interpolation. (otherwise this will result in empty/trivial interpolation)
 */
class CMirror : public CInterpolator {
public:

  /*!
   * \brief Constructor of the class.
   * \param[in] geometry_container
   * \param[in] config - config container
   * \param[in] iZone - First zone
   * \param[in] jZone - Second zone
   *
   * Data is set in geometry[targetZone]
   *
   */
  CMirror(CGeometry ***geometry_container, CConfig **config, unsigned int iZone, unsigned int jZone);

  /*!
   * \brief Destructor of the class.
   */
  ~CMirror(void);

  /*!
   * \brief Set up transfer matrix defining relation between two meshes
   * \param[in] config - Definition of the particular problem.
   */
  void Set_TransferCoeff(CConfig **config);
  
};

/*!
 * \brief Sliding mesh approach
  */
class CSlidingmesh : public CInterpolator {
public:

  /*!
   * \brief Constructor of the class.
   * \param[in] geometry - Geometrical definition of the problem.
   * \param[in] config - Definition of the particular problem.
   * \param[in] iZone - index of the donor zone
   * \param[in] jZone - index of the target zone
   */
  CSlidingmesh(CGeometry ***geometry_container, CConfig **config, unsigned int iZone, unsigned int jZone);

  /*!
   * \brief Destructor of the class.
   */
  ~CSlidingmesh(void);

  /*!
   * \brief Set up transfer matrix defining relation between two meshes
   * \param[in] config - Definition of the particular problem.
   */
  void Set_TransferCoeff(CConfig **config);
  
  /*!
   * \brief For 3-Dimensional grids, build the dual surface element
   * \param[in] geometry - geometry where the node cell belongs
   * \param[in] centralNode - label of the vertex around which the dual surface element is built
   * \param[in] markID   - node centered cell index
   * \param[in] element  - double array where element node coordinates will be stored
   * \param[in] output   - number of nodes in the element
   */  
  int Build_3D_surface_element(CGeometry *geometry, unsigned long centralNode, unsigned long markID, su2double** element);
  
  /*!
   * \brief compute distance between 2 points
   * \param[in] point_i
   * \param[in] point_i
   */
  su2double PointsDistance(su2double *point_i, su2double *point_j);
   
  /*!
   * \brief For 2-Dimensional grids, compute intersection length of two segments projected along a given direction
   * \param[in] A1 - first  point of segment A
   * \param[in] A2 - second point of segment A
   * \param[in] B1 - first  point of segment B
   * \param[in] B2 - second point of segment B
   * \param[in] Direction - along which segments are projected
   */
  su2double Compute_Intersection_2D(su2double* A1, su2double* A2, su2double* B1, su2double* B2, su2double* Direction);
  
  /*!
   * \brief For 2-Dimensional grids, find the label of a vertex next to the current vertex, following a given direction
   * \param[in] geometry - geometry where the node cell belongs
   * \param[in] PreviousVertex - label of the previous vertex so that the direction is specified
   * \param[in] VertexID - node centered cell index
   * \param[in] markID   - node centered cell index
   */
  int FindNextNode_2D(CGeometry *geometry, int PreviousNode, unsigned long NodeID, unsigned long markID);
  
  /*!
   * \brief For 3-Dimensional grids, compute intersection area between two triangle projected on a given plane
   * \param[in] A1 - first  point of triangle A
   * \param[in] A2 - second point of triangle A
   * \param[in] A3 - third  point of triangle A
   * \param[in] B1 - first  point of triangle B
   * \param[in] B2 - second point of triangle B
   * \param[in] B3 - third  point of triangle B
   * \param[in] Direction - vector normal to projection plane
   */
  su2double Compute_Triangle_Intersection(su2double* A1, su2double* A2, su2double* A3, su2double* B1, su2double* B2, su2double* B3, su2double* Direction);
  
  /*!
   * \brief For 3-Dimensional grids, compute intersection area between two triangle projected on a given plane
   * P1 from triangle P MUST be inside triangle Q, points order doesn't matter
   * \param[in] P1 - first  point of triangle A
   * \param[in] P2 - second point of triangle A
   * \param[in] P3 - third  point of triangle A
   * \param[in] Q1 - first  point of triangle B
   * \param[in] Q2 - second point of triangle B
   * \param[in] Q3 - third  point of triangle B
   */
  su2double ComputeIntersectionArea( su2double* P1, su2double* P2, su2double* P3, su2double* Q1, su2double* Q2, su2double* Q3 );
  
  /*!
   * \brief For 2-Dimensional grids, check whether, and compute, two lines are intersecting
   * \param[in] A1 - first  defining first line
   * \param[in] A2 - second defining first line
   * \param[in] B1 - first  defining second line
   * \param[in] B2 - second defining second line
   * \param[in] IntersectionPoint - Container for intersection coordinates
   */
  void ComputeLineIntersectionPoint( su2double* A1, su2double* A2, su2double* B1, su2double* B2, su2double* IntersectionPoint );
  
  /*!
   * \brief For N-Dimensional grids, check whether a point is inside a triangle specified by 3 T points
   * \param[in] Point - query point
   * \param[in] T1 - first  point of triangle T
   * \param[in] T2 - second point of triangle T
   * \param[in] T3 - third  point of triangle T
   */
  bool CheckPointInsideTriangle(su2double* Point, su2double* T1, su2double* T2, su2double* T3);
};


