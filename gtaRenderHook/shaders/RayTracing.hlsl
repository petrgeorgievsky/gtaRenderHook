struct Sphere
{
    float3 position;
    float radius;
};

struct Ray
{
    float4 origin;
    float4 direction;
};



bool RaySphereIntersection(Ray r, Sphere s)
{
    float t0, t1, temp;

    float3 to_sphere = s.position - r.origin.xyz;
    float tca = dot(r.direction.xyz, to_sphere);
        // if (tca < 0) return false;
    float d2 = dot(to_sphere, to_sphere) - tca * tca;
    if (d2 > s.radius)
        return false;
    float thc = sqrt(s.radius - d2);
    t0 = tca - thc;
    t1 = tca + thc;

    if (t0 > t1)
    {
        temp = t0;
        t0 = t1;
        t1 = temp;
    }
 
    if (t0 < 0)
    {
        t0 = t1; // if t0 is negative, let's use t1 instead 
        if (t0 < 0)
            return false; // both t0 and t1 are negative 
    }
 
    //t = t0;
 
    return true;

}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    Sphere s;
    s.position = float3(100, 100, 100);
    s.radius = 50;
    float imageWidth = 512;
    float imageHeight = 512;
    float M_PI = 3.14;
    float imageAspectRatio = imageWidth / imageHeight;
    float fov = 45;

    float Px = (2 * ((DTid.x + 0.5) / imageWidth) - 1) * tan(fov / 2 * M_PI / 180) * imageAspectRatio;
    float Py = (1 - 2 * ((DTid.y + 0.5) / imageHeight) * tan(fov / 2 * M_PI / 180));
    Ray r;
    r.origin = float4(0, 0, 0, 0);
    r.direction = float4(Px, Py, -1, 0);
}