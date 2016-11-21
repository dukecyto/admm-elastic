// Copyright (c) 2016, University of Minnesota
// 
// lbfgssolver Uses the BSD 2-Clause License (http://www.opensource.org/licenses/BSD-2-Clause)
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other materials
//    provided with the distribution.
// THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY OF MINNESOTA, DULUTH OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
// IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef ADMM_SYSTEM_H
#define ADMM_SYSTEM_H 1

#include "Force.hpp"
#include "ExplicitForce.hpp"
#include <Eigen/SparseCholesky>
#include <Eigen/IterativeLinearSolvers>

namespace admm {

class System {
public:
	System() : timestep_s(0.04), elapsed_s(0.0), m_verbose(1), initialized(false) {}

	double timestep_s; // timestep in seconds
	double elapsed_s; // accumulated time in seconds

	// Per-node (x3) data (for x, y, and z)
	Eigen::VectorXd m_x; // node positions, scaled x3
	Eigen::VectorXd m_v; // node velocities, scaled x3
	Eigen::VectorXd m_masses; // node masses, scaled x3

	std::vector< std::shared_ptr<ExplicitForce> > explicit_forces; // forces applied explicitly
	std::vector< std::shared_ptr<Force> > forces; // minimized (implicit)

	// Adds nodes to the system.
	// Returns the current total number of nodes after insert.
	// Assumes x and m are scaled x3.
	int add_nodes( Eigen::VectorXd x, Eigen::VectorXd m );

	// Returns true on success.
	// Computes global matrices and should only be called once
	// after all nodes have been added to the system. Once called,
	// no more nodes can be added or removed.
	bool initialize( double timestep_ );

	// Performs a system step without any capture/prints
	bool step( int solver_iters=10 );

	// Set the print output level.
	// Each higher number includes the prints of lower numbers.
	// 0=silent, 1=minimal, 2=everything
	void verbose( unsigned int v ){ m_verbose=v; }

	// You can change the weights at runtime.
	// To do so, adjust the weight value associated with whatever forces
	// you want to change. Then, call this function. However, this
	// means the system has to be recomputed, so do it sparingly.
	void recompute_weights();

	// Adds a callback function that is executed at the beginning of a step.
	// This is helpful for things like recording residuals, updating anchor control points, etc...
	void add_step_callback( std::function<void ( admm::System* )> &cb ){ step_callbacks.push_back( cb ); }

protected:
	// Users can add callbacks with the "add_step_callback" function
	// that are executed during the solve.
	std::vector< std::function<void ( admm::System* )> > step_callbacks;

	// Settings
	unsigned int m_verbose;
	bool initialized;

	// Global matrices
	Eigen::SparseMatrix<double> m_D; // "reduction" matrix
	Eigen::VectorXd m_W_diag; // diagonal of the weight matrix

	// Solver variables computed in initialize
	Eigen::SparseMatrix<double> solver_dt2_Dt_Wt_W;
	// Eigen::SimplicialLDLT< Eigen::SparseMatrix<double> > solver;
	Eigen::ConjugateGradient< Eigen::SparseMatrix<double>, Eigen::Upper|Eigen::Lower > solver;

	// These variables don't need to be class members, but
	// are stored as such to avoid reallocation. Otherwise it
	// becomes noticeably slower for large systems.
	Eigen::VectorXd solver_termB;
	Eigen::VectorXd Dx;
	Eigen::VectorXd curr_u; // admm dual
	Eigen::VectorXd curr_z; // admm primal

	// Output variables for printing progress, runtime, etc...
	void print_progress( int iter, int max_iters );

}; // end class system


} // end namespace admm

#endif




