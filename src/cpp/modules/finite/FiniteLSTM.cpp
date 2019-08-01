// FiniteLSTM.cpp : Defines the exported functions for the DLL.
#include "FiniteLSTM.h"
#include "../../shared/lstm.h"
#include "finite.h"

// This function must be called before any other function.
void FiniteLSTM::prepare(LSTMInput&& input)
{
    this->input = input;
    int Jcols = 8 * input.l * input.b + 3 * input.b;
    state = std::vector<double>(input.state.size());
    result = { 0, std::vector<double>(Jcols) };
}

LSTMOutput FiniteLSTM::output()
{
    return result;
}

void FiniteLSTM::calculateObjective(int times)
{
    for (int i = 0; i < times; ++i) {
        state = input.state;
        lstm_objective(input.l, input.c, input.b, input.main_params.data(), input.extra_params.data(), state, input.sequence.data(), &result.objective);
    }
}

void FiniteLSTM::calculateJacobian(int times)
{
    for (int i = 0; i < times; ++i) {
        state = input.state;
        finite_differences<double>([&](double* main_params_in, double* loss) {
            lstm_objective(input.l, input.c, input.b, main_params_in,
                input.extra_params.data(), state, input.sequence.data(), loss);
            }, input.main_params.data(), input.main_params.size(), &result.objective, 1, result.gradient.data());

        finite_differences<double>([&](double* extra_params_in, double* loss) {
            lstm_objective(input.l, input.c, input.b, input.main_params.data(),
                extra_params_in, state, input.sequence.data(), loss);
            }, input.extra_params.data(), input.extra_params.size(), &result.objective, 1, &result.gradient.data()[2 * input.l * 4 * input.b]);
    }
}

extern "C" DLL_PUBLIC ITest<LSTMInput, LSTMOutput>*  GetLSTMTest()
{
    return new FiniteLSTM();
}