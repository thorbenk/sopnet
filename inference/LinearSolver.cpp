#include <util/Logger.h>
#include <util/foreach.h>
#include "LinearSolver.h"

static logger::LogChannel linearsolverlog("linearsolverlog", "[LinearSolver] ");

LinearSolver::LinearSolver(const LinearSolverBackendFactory& backendFactory) {

	registerInput(_objective, "objective");
	registerInput(_linearConstraints, "linear constraints");
	registerInput(_parameters, "parameters");
	registerOutput(_solution, "solution");

	// create solver backend
	_solver = backendFactory.createLinearSolverBackend();
}

LinearSolver::~LinearSolver() {

	delete _solver;
}

void
LinearSolver::updateOutputs() {

	updateLinearProgram();

	solve();
}

void
LinearSolver::updateLinearProgram() {

	_solver->initialize(
			getNumVariables(),
			(_parameters ? _parameters->getVariableType() : Continuous));

	_solver->setObjective(*_objective);

	_solver->setConstraints(*_linearConstraints);
}

void
LinearSolver::solve() {

	double value;

	std::string message;

	if (_solver->solve(*_solution, value, message))
		LOG_USER(linearsolverlog) << "optimal solution found" << std::endl;
	else
		LOG_ERROR(linearsolverlog) << "error: " << message << std::endl;
}

unsigned int
LinearSolver::getNumVariables() {

	unsigned int numVars = 0;

	// number of vars in the objective
	numVars = std::max<unsigned int>(numVars, _objective->getCoefficients().size());

	// numver of vars in the constraints
	typedef std::pair<unsigned int, double> lin_coef_type;
	foreach (const LinearConstraint& constraint, *_linearConstraints)
		foreach (const lin_coef_type& pair, constraint.getCoefficients())
			numVars = std::max(numVars, pair.first);

	return numVars;
}
