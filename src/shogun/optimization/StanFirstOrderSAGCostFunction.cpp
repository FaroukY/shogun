/*
 * This software is distributed under BSD 3-clause license (see LICENSE file).
 *
 * Authors: Elfarouk
 */

#include <shogun/optimization/StanFirstOrderSAGCostFunction.h>
#include <shogun/base/range.h>
#include <shogun/mathematics/Math.h>
using namespace shogun;
using stan::math::var;
using std::function;
using Eigen::Matrix;
using Eigen::Dynamic;

StanFirstOrderSAGCostFunction::StanFirstOrderSAGCostFunction(
    SGMatrix<float64_t> X, SGMatrix<float64_t> y,
    StanVector* trainable_parameters,
    StanFunctionsVector<int32_t>* cost_for_ith_point,
    FunctionReturnsStan<StanVector*>* total_cost)
{
	REQUIRE(X.size() > 0, "Empty X provided");
	REQUIRE(y.size() > 0, "Empty y provided");
	auto num_of_variables = trainable_parameters->rows();
	REQUIRE(
	    num_of_variables > 0, "Provided %d variables in the parameters, more "
	                          "than 0 parameters required",
	    num_of_variables);
	REQUIRE(cost_for_ith_point != NULL, "Cost for ith point is not provided");
	REQUIRE(total_cost != NULL, "Total cost function is not provided");
	m_X = X;
	m_y = y;
	m_trainable_parameters = trainable_parameters;
	m_cost_for_ith_point = std::move(cost_for_ith_point);
	m_total_cost = std::move(total_cost);
}

void StanFirstOrderSAGCostFunction::set_training_data(
    SGMatrix<float64_t> X_new, SGMatrix<float64_t> y_new)
{
	REQUIRE(X_new.size() > 0, "Empty X provided");
	REQUIRE(y_new.size() > 0, "Empty y provided");
	this->m_X = X_new;
	this->m_y = y_new;
}

StanFirstOrderSAGCostFunction::~StanFirstOrderSAGCostFunction()
{
}

void StanFirstOrderSAGCostFunction::begin_sample()
{
	m_index_of_sample = -1;
}

bool StanFirstOrderSAGCostFunction::next_sample()
{
	++m_index_of_sample;
	return m_index_of_sample < get_sample_size();
}

SGVector<float64_t> StanFirstOrderSAGCostFunction::get_gradient()
{
	auto num_of_variables = m_trainable_parameters->rows();
	REQUIRE(
	    num_of_variables > 0,
	    "Number of sample must be greater than 0, you provided no samples");

	var f_i = (*m_cost_for_ith_point)(m_index_of_sample, 0)(m_index_of_sample);

	stan::math::set_zero_all_adjoints();
	f_i.grad();

	SGVector<float64_t>::EigenVectorXt gradients =
	    m_trainable_parameters->unaryExpr(
	        [](stan::math::var x) -> float64_t { return x.adj(); });
	// clone needed because gradients is local variable
	return SGVector<float64_t>(gradients).clone();
}

float64_t StanFirstOrderSAGCostFunction::get_cost()
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

index_t StanFirstOrderSAGCostFunction::get_sample_size()
{
	return m_X.num_cols;
}

SGVector<float64_t> StanFirstOrderSAGCostFunction::get_average_gradient()
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
