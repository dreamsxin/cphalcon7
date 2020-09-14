#include "carray.h"
#include "conversion_utils.h"

/*
 * Converts an axis parameter into an ndim-length C-array of
 * boolean flags, True for each axis specified.
 *
 * If obj is None or NULL, everything is set to True. If obj is a tuple,
 * each axis within the tuple is set to True. If obj is an integer,
 * just that axis is set to True.
 */
int
CArray_ConvertMultiAxis(int *axis_in, int ndim, int *out_axis_flags)
{
    /* INT_MAX means all of the axes */
    if (*axis_in == INT_MAX || axis_in == NULL) {
        memset(out_axis_flags, 1, ndim);
        return CARRAY_SUCCEED;
    } 
    /* Try to interpret axis as an integer */
    else {
        int axis;
        axis = axis_in[0];
        
        memset(out_axis_flags, 0, ndim);

        /*
         * Special case letting axis={-1,0} slip through for scalars,
         * for backwards compatibility reasons.
         */
        if (ndim == 0 && (axis == 0 || axis == -1)) {
            return CARRAY_SUCCEED;
        }

        if (check_and_adjust_axis(&axis, ndim) < 0) {
            return CARRAY_FAIL;
        }

        out_axis_flags[axis] = 1;
        
        return CARRAY_SUCCEED;
    }
}