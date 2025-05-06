#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include <CL/cl_gl.h>

inline cl_int clGetGLContextInfoKHR_safe(
    const cl_context_properties *properties,
    cl_gl_context_info param_name,
    size_t param_value_size,
    void *param_value,
    size_t *param_value_size_ret)
{
    static auto func = reinterpret_cast<cl_int (*)(const cl_context_properties *, cl_gl_context_info, size_t, void *, size_t *)>(
        clGetExtensionFunctionAddress("clGetGLContextInfoKHR"));
    
    if (!func)
        return CL_INVALID_OPERATION;

    return func(properties, param_name, param_value_size, param_value, param_value_size_ret);
}

#pragma GCC diagnostic pop