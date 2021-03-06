#include <math.h>
#include "../thnets.h"

int nnload_LogSoftMax(struct module *mod, struct nnmodule *n)
{
	mod->type = MT_LogSoftMax;
	mod->updateOutput = nn_LogSoftMax_updateOutput;
	return 0;
}

THFloatTensor *nn_LogSoftMax_updateOutput(struct module *module, THFloatTensor *input)
{
	THFloatTensor *output = module->output;
	float *input_data, *output_data;
	long nframe = 0, dim = 0, stride = 0;
	long t;

	if(input->nDimension == 1)
	{
		nframe = 1;
		dim = input->size[0];
		stride = 1;
	}
	else if(input->nDimension == 2)
	{
		nframe = input->size[0];
		dim = input->size[1];
		stride = 1;
	}
	else if(input->nDimension == 3)
	{
		nframe = 1;
		dim = input->size[0];
		stride = input->size[1]*input->size[2];
	}
	else if(input->nDimension == 4)
	{
		nframe = input->size[0];
		dim = input->size[1];
		stride = input->size[2]*input->size[3];
	}
	else
		THError("1D, 2D, 3D or 4D tensor expected");

	THFloatTensor_resizeAs(output, input);

	input_data = THFloatTensor_data(input);
	output_data = THFloatTensor_data(output);

#pragma omp parallel for private(t)
	for(t = 0; t < stride*nframe; t++)
	{
		float *input_ptr = input_data + (t/stride)*dim*stride + t % stride;
		float *output_ptr = output_data + (t/stride)*dim*stride + t % stride;

		float logSum = 0;
		float inputMax = -THInf;

		long d;
		for(d = 0; d < dim; d++) {
			if (input_ptr[d*stride] >= inputMax) inputMax = input_ptr[d*stride];
		}

		for(d = 0; d < dim; d++)
			logSum += exp(input_ptr[d*stride] - inputMax);
		logSum = inputMax + log(logSum);

		for(d = 0; d < dim; d++)
			output_ptr[d*stride] = input_data[d*stride] - logSum;
	}

	return output;
}
