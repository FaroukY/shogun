/*
 * This software is distributed under BSD 3-clause license (see LICENSE file).
 *
 * Authors: Elfarouk
 */

#include <shogun/optimization/FirstOrderSAGCostFunctionInterface.h>
#include <shogun/base/range.h>
#include <shogun/mathematics/Math.h>
using namespace shogun;
using stan::math::var;
using std::function;
using Eigen::Matrix;
using Eigen::Dynamic;

FirstOrderSAGCostFunctionInterface::FirstOrderSAGCostFunctionInterface(
    SGMatrix<float64_t> X, SGMatrix<float64_t> y,
    StanVector* trainable_parameters,
    Matrix<function<var(int32_t)>, Dynamic, 1>* cost_for_ith_point,
    function<var(StanVector*)>* total_cost)
{
	m_X = X;
	m_y = y;
	m_trainable_parameters = trainable_parameters;
	m_cost_for_ith_point = cost_for_ith_point;
	m_total_cost = total_cost;
}

void FirstOrderSAGCostFunctionInterface::set_training_data(
    SGMatrix<float64_t> X_new, SGMatrix<float64_t> y_new)
{
	REQUIRE(!X_new.equals(SGMatrix<float64_t>()), "Empty X provided");
	REQUIRE(!y_new.equals(SGMatrix<float64_t>()), "Empty y provided");
	this->m_X = X_new;
	this->m_y = y_new;
}

void FirstOrderSAGCostFunctionInterface::set_trainable_parameters(
    StanVector* new_params)
{
	REQUIRE(new_params, "The trainable parameters must be provided");
	if (this->m_trainable_parameters != new_params)
	{
		this->m_trainable_parameters = new_params;
	}
}

void FirstOrderSAGCostFunctionInterface::set_ith_cost_function(
    Matrix<function<var(int32_t)>, Dynamic, 1>* new_cost_f)
{
	REQUIRE(new_cost_f, "The cost function must be a vector of stan variables");
	if (this->m_cost_for_ith_point != new_cost_f)
	{
		this->m_cost_for_ith_point = new_cost_f;
	}
}

void FirstOrderSAGCostFunctionInterface::set_cost_function(
    function<var(StanVector*)>* total_cost)
{
	REQUIRE(
	    total_cost,
	    "The total cost function must be a function returning a stan variable");
	if (this->m_total_cost != total_cost)
	{
		this->m_total_cost = total_cost;
	}
}

FirstOrderSAGCostFunctionInterface::~FirstOrderSAGCostFunctionInterface()
{
}

void FirstOrderSAGCostFunctionInterface::begin_sample()
{
	m_index_of_sample = 0;
}

bool FirstOrderSAGCostFunctionInterface::next_sample()
{
	if (m_index_of_sample >= get_sample_size())
		return false;
	++m_index_of_sample;
	return true;
}

SGVector<float64_t> FirstOrderSAGCostFunctionInterface::get_gradient()
{
	auto num_of_variables = m_trainable_parameters->rows();
	REQUIRE(
	    num_of_variables > 0,
	    "Number of sample must be greater than 0, you provided no samples");

	// SGVector<float64_t> gradients(num_of_variables);

	var f_i = (*m_cost_for_ith_point)(m_index_of_sample, 0)(m_index_of_sample);

	stan::math::set_zero_all_adjoints();
	f_i.grad();

	SGVector<float64_t>::EigenVectorXt gradients =
	    m_trainable_parameters->unaryExpr(
	        [](stan::math::var x) -> float64_t { return x.adj(); });
	// clone needed because gradients is local variable
	return SGVector<float64_t>(gradients).clone();
}

float64_t FirstOrderSAGCostFunctionInterface::get_cost()
{
	auto n = get_sample_size();
	StanVector cost_argument(n);

	for (auto i : range(n))
	{
		cost_argument(i, 0) = (*m_cost_for_ith_point)(i, 0)(i);
	}
	var cost = (*m_total_cost)(&cost_argument);
	return cost.val();
}

index_t FirstOrderSAGCostFunctionInterface::get_sample_size()
{
	return m_X.num_cols;
}

SGVector<float64_t> FirstOrderSAGCostFunctionInterface::get_average_gradient()
{
	int32_t params_num = m_trainable_parameters->rows();
	SGVector<float64_t> average_gradients(params_num);

	auto old_index_sample = m_index_of_sample;
	auto n = get_sample_size();
	REQUIRE(
	    n > 0,
	    "Number of sample must be greater than 0, you provided no samples");

	for (index_t i = 0; i < n; ++i)
	{
		m_index_of_sample = i;
		average_gradients += get_gradient();
	}
	average_gradients.scale(1.0 / n);
	m_index_of_sample = old_index_sample;
	return average_gradients;
}
