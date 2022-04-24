/*
 * Copyright (c) 2014 Matt Hall <mtjhall@alumni.uvic.ca>
 * 
 * This file is part of MoorDyn.  MoorDyn is free software: you can redistribute 
 * it and/or modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * 
 * MoorDyn is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with MoorDyn.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LINE_H
#define LINE_H

#include "Misc.h"

using namespace std;

// here is the numbering scheme (N segments per line):
//   [connect (node 0)]  --- segment 0 --- [ node 1 ] --- seg 1 --- [node2] --- ... --- seg n-2 --- [node n-1] --- seg n-1 ---  [connect (node N)]

//class Connection;
class Waves;

class Line 
{
	
	// ENVIRONMENTAL STUFF	
	EnvCond *env;  // pointer to global struct that holds environmental settings
	Waves *waves;  // pointer to global Waves object
	
		
	// LINE STUFF
    
	// parameters
	LineProps props;	
	int N; // number of line nodes 
	moordyn::real UnstrLen;
	moordyn::real d;		// line diameter
	moordyn::real rho;		// line linear density
	moordyn::real E;		// line elasticity modulus [Pa] 
	moordyn::real EI;		// line bending stiffness [Nm^2] <<<<<<<< need to figure out how to load in through input file (where to put)
	moordyn::real c;		// line axial internal damping coefficient [Ns]
	moordyn::real cI;		// line bending internal damping coefficient [??]
	moordyn::real Can;
	moordyn::real Cat;
	moordyn::real Cdn;
	moordyn::real Cdt;
	
	moordyn::real BAin;		// line axial internal damping coefficient input (before proceessing)
	
	moordyn::real A; // line cross-sectional area to pre-compute
	
	int nEApoints = 0; // number of values in stress-strain lookup table (0 means using constant E)
	moordyn::real stiffXs[nCoef]; // x array for stress-strain lookup table (up to nCoef)
	moordyn::real stiffYs[nCoef]; // y array for stress-strain lookup table
	int nCpoints;        // number of values in stress-strainrate lookup table (0 means using constant c)
	moordyn::real dampXs[nCoef]; // x array for stress-strainrate lookup table (up to nCoef)
	moordyn::real dampYs[nCoef]; // y array for stress-strainrate lookup table
	int nEIpoints = 0; // number of values in stress-strain lookup table (0 means using constant E)
	moordyn::real bstiffXs[nCoef]; // x array for stress-strain lookup table (up to nCoef)
	moordyn::real bstiffYs[nCoef]; // y array for stress-strain lookup table
	
	
	// kinematics
	std::vector<vec> r;             // node positions [i][x/y/z]
	std::vector<vec> rd;            // node velocities [i][x/y/z]
	double **q;             // unit tangent vectors for each node
	double **qs;            // unit tangent vectors for each segment (used in bending calcs)
	double *l;              // line unstretched segment lengths
	double *lstr;           // stretched lengths
	double *ldstr;          // rate of stretch
	double *Kurv;           // curvatures at node points (1/m)
	
	double ***M;            // node mass + added mass matrix
	double Mext[3];         // net moment from attached lines at either end
	double *V;              // line segment volume	
				
	// forces 
	double **T;             // segment tensions
	double **Td;            // segment damping forces
	double **Bs;            // bending stiffness forces
	double **W;             // node weight 	
	double **Dp;            // node drag (transverse)
	double **Dq;            // node drag (axial)
	double **Ap;            // node added mass forcing (transverse)
	double **Aq;            // node added mass forcing (axial)
	double **B;             // node bottom contact force	
	double **Fnet;          // total force on node  <<<<<<< might remove this for Rods
		
	// wave things
	double *F; 		        // VOF scalar for each segment (1 = fully submerged, 0 = out of water)
	double *zeta;           // free surface elevation
	double *PDyn;           // dynamic pressure
	double **U;             // wave velocities	
	double **Ud;            // wave accelerations	
	
	
	// time
	moordyn::real t;               // simulation time
	moordyn::real t0;              // simulation time current integration was started at (used for BC function)
	moordyn::real tlast;
	
	// end conditions
	int endTypeA;           // type of connection at end A: 0=pinned to Connection, 1=cantilevered to Rod.
	int endTypeB;
	double endMomentA[3];   // moment at end A from bending, to be applied on attached Rod/Body
	double endMomentB[3];

	// file stuff	
	ofstream * outfile;     // if not a pointer, caused odeint system initialization error during compilation
	string channels;
	
	// data structures for precalculated nodal water kinematics if applicable
	double **zetaTS;        // time series of wave elevations above each node
	double **FTS;
	double ***UTS;
	double ***UdTS;
	int ntWater;            // number of water kinematics time steps
	moordyn::real dtWater;         // water kinematics time step size (s)

//	int ts0; 				// time step index used for interpolating wave kinematics time series data (put here so it's persistent) ????
	

public:
 	int number; // line "number" id
	
	int WaterKin;  // flag indicating whether wave/current kinematics will be considered for this linec
	// 0: none, or use value set externally for each node of the object; 1: interpolate from stored; 2: call interpolation function from global Waves grid

 
 	// unique to Line
	//Connection* AnchConnect;  // pointer to anchor connection
	//Connection* FairConnect;  // pointer to fairlead connection
 
	void setup(int number, LineProps *props_in, double UnstrLen_in, int NumNodes, 
	//	Connection& AnchConnect_in, Connection& FairConnect_in,
		shared_ptr<ofstream> outfile_pointer, string channels_in);
	
	void initializeLine(double* X );

	void setEnv(EnvCond *env_in, Waves* waves_in);

	double getNodeTen(int i);
	
	int getNodePos(int i, double pos[3]);
	void getNodeCoordinates(double r_out[]);
	void setNodeWaveKin(double U_in[], double Ud_in[]);
	
	double GetLineOutput(OutChanProps outChan);
	
	void storeWaterKin(int nt, double dt, double **zeta_in, double **f_in, double ***u_in, double ***ud_in);
	
	void getFASTtens(float* FairHTen, float* FairVTen, float* AnchHTen, float* AnchVTen);

	void getEndStuff(double Fnet_out[3], double Moment_out[3], double M_out[3][3], int topOfLine);

	int getN(); // returns N (number of segments)
	
	void setupWaves(vector<double> Ucurrent_in, float dt_in);
		
	double getNonlinearE(double l_stretched, double l_unstretched);
	double getNonlinearC(double ld_stretched, double l_unstretched);
		
	void scaleDrag(double scaler);	
	void setTime(double time);
	
	void setState( const double* X, const double time);
	
	void setEndState(double r_in[3], double rd_in[3], int topOfLine);
	void setEndState(vector<double> &r_in, vector<double> &rd_in, int topOfLine);
	
	void setEndOrientation(double *qin, int topOfLine, int rodEndB);
	
	void getEndSegmentInfo(double q_EI_dl[3], int topOfLine, int rodEndB);
	void getEndSegmentInfo(double qEnd[3], double *EIout, double *dlout, int topOfLine);
	
	void getStateDeriv(double* Xd, const double dt);
	
	void doRHS( const double* X,  double* Xd, const double time, const double dt);

	//void initiateStep(vector<double> &rFairIn, vector<double> &rdFairIn, double time);
		
	void Output(double );
	
	~Line();

#ifdef USEGL	
	void drawGL(void);
	void drawGL2(void);
#endif
};

#endif


