kernel void add(
	global const float *restrict a,
	global const float *restrict b,
	global float *restrict c)
{
    int gid = get_global_id(0);
	c[gid] = a[gid] + b[gid];
}
