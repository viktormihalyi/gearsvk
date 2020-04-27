#version 450

layout (std140, binding = 0) uniform Time {
    float time;
} time;

layout (std140, binding = 3) uniform Camera {
    mat4 VP;
    mat4 rayDirMatrix;
    vec3 position;
} camera;

layout (binding = 1) uniform sampler2D agy2dSampler;
layout (binding = 2) uniform sampler3D agySampler;

layout (location = 0) in vec2 uv;
layout (location = 1) in vec3 rayDirection;

layout (location = 2) out vec4 presented;
layout (location = 0) out vec4 copy[2];




#define EPS 1e-5
#define FLT_MAX 3.402823466e+38
#define FLT_MIN 1.175494351e-38
#define DBL_MAX 1.7976931348623158e+308
#define DBL_MIN 2.2250738585072014e-308



float SurfaceIntersection (vec3 rayStart, vec3 rayDir, vec3 surfacePoint, vec3 surfaceNormal)
{
    float t = (dot (surfaceNormal, surfacePoint) - dot (surfaceNormal, rayStart)) / dot (rayDir, surfaceNormal);
    return t;
}


bool IsInsideBox (vec3 p, vec3 boxMiddle, float boxRadius)
{
    boxRadius += EPS;
    vec3 delta = p - boxMiddle;
    return abs (delta.x) <= boxRadius && abs (delta.y) <= boxRadius && abs (delta.z) <= boxRadius;
}

vec2 Box2Intersection (vec3 rayStart, vec3 rayDir, vec3 middle, float radius)
{
    vec3 normalTop    = vec3 (0, 0, +1);
    vec3 normalBottom = vec3 (0, 0, -1);
    vec3 normalRight  = vec3 (0, +1, 0);
    vec3 normalLeft   = vec3 (0, -1, 0);
    vec3 normalFront  = vec3 (+1, 0, 0);
    vec3 normalBack   = vec3 (-1, 0, 0);

    float top    = SurfaceIntersection (rayStart, rayDir, middle + normalTop    * radius, normalTop);
    float bottom = SurfaceIntersection (rayStart, rayDir, middle + normalBottom * radius, normalBottom);
    float right  = SurfaceIntersection (rayStart, rayDir, middle + normalRight  * radius, normalRight);
    float left   = SurfaceIntersection (rayStart, rayDir, middle + normalLeft   * radius, normalLeft);
    float front  = SurfaceIntersection (rayStart, rayDir, middle + normalFront  * radius, normalFront);
    float back   = SurfaceIntersection (rayStart, rayDir, middle + normalBack   * radius, normalBack);

    vec3 topIntersectionPoint    = rayStart + rayDir * top;
    vec3 bottomIntersectionPoint = rayStart + rayDir * bottom;
    vec3 rightIntersectionPoint  = rayStart + rayDir * right;
    vec3 leftIntersectionPoint   = rayStart + rayDir * left;
    vec3 frontIntersectionPoint  = rayStart + rayDir * front;
    vec3 backIntersectionPoint   = rayStart + rayDir * back;

    float inT  = FLT_MAX;
    float outT = FLT_MAX;

    // find first intersection

    if (top    < inT && IsInsideBox (topIntersectionPoint,    middle, radius)) { inT = top; } 
    if (bottom < inT && IsInsideBox (bottomIntersectionPoint, middle, radius)) { inT = bottom; } 
    if (right  < inT && IsInsideBox (rightIntersectionPoint,  middle, radius)) { inT = right; } 
    if (left   < inT && IsInsideBox (leftIntersectionPoint,   middle, radius)) { inT = left; } 
    if (front  < inT && IsInsideBox (frontIntersectionPoint,  middle, radius)) { inT = front; } 
    if (back   < inT && IsInsideBox (backIntersectionPoint,   middle, radius)) { inT = back; } 

    // find second intersection
    
    if (inT < top    && top    < outT && IsInsideBox (topIntersectionPoint,    middle, radius)) { outT = top; } 
    if (inT < bottom && bottom < outT && IsInsideBox (bottomIntersectionPoint, middle, radius)) { outT = bottom; } 
    if (inT < right  && right  < outT && IsInsideBox (rightIntersectionPoint,  middle, radius)) { outT = right; } 
    if (inT < left   && left   < outT && IsInsideBox (leftIntersectionPoint,   middle, radius)) { outT = left; } 
    if (inT < front  && front  < outT && IsInsideBox (frontIntersectionPoint,  middle, radius)) { outT = front; } 
    if (inT < back   && back   < outT && IsInsideBox (backIntersectionPoint,   middle, radius)) { outT = back; } 

    return vec2 (inT, outT);
}


float TriangleIntersection (vec3 rayStart, vec3 rayDir, vec3 p1, vec3 p2, vec3 p3)
{
    vec3 normal = normalize (cross (p2 - p1, p3 - p1));

    float t = dot (p1 - rayStart, normal) / dot (rayDir, normal);
    //if (t < 0) {
    //    return -1.f;
    //}

    vec3 P = rayStart + rayDir * t;

    if (dot (cross (p2 - p1, P - p1), normal) > 0 &&
        dot (cross (p3 - p2, P - p2), normal) > 0 &&
        dot (cross (p1 - p3, P - p3), normal) > 0) {

        return t;
    }

    return -1.f;
}


void main ()
{
    vec4 result = vec4 (vec3 (uv, 1.f), 1);
    presented = vec4 (texture (agySampler, vec3(uv, mod (time.time, fract (time.time*0.1f))  )).rrr, 1);
    //presented = result;
    //presented = vec4 (normalize (rayDirection) * 0.5 + 0.5, 1);

    vec3 normRayDir = normalize (rayDirection);
    //vec3 rayPos = camera.position;
    //float rayStep = 0.1f;
    //float val = 0.f;
//
//    vec3 sphereCenter = vec3 (0, 0, 0);
//    float sphereRadius = 1.f;
//
//    float minDist = 99999.f;
//    //presented = vec4 (vec3 (distance (rayPos, sphereCenter) - sphereRadius), 1.f);
//
//    for (int i = 0; i < 64; ++i) {
//        float dist = distance (rayPos, sphereCenter) - sphereRadius;
//        if (dist < minDist) {
//            minDist = dist;
//        }
//        //val += texture (agySampler, rayPos).r;
//
//        rayPos += normRayDir * rayStep;
//    }
//
    //presented = vec4 (vec3 (minDist), 1);
    //presented = vec4 (normalize (rayDirection) * 0.5 + 0.5, 1);

    //float t = SurfaceIntersection (camera.position, normRayDir, vec3 (0, 0, -1), vec3 (0, 0, 1));
    //t = TriangleIntersection (camera.position, normRayDir, vec3 (1, 0, 0), vec3 (0, 0, 1), vec3 (0, 1, 0));
    vec2 bt = Box2Intersection (camera.position, normRayDir, vec3 (0.5f, 0.5f, 0.5f), 0.5f);
    bool hit = (bt.x > 0.f  && bt.x < FLT_MAX && bt.y > 0.f && bt.y < FLT_MAX);
    bool hitFromInside = !hit && (bt.x > 0.f  && bt.x < FLT_MAX);

    presented = vec4 (vec3 (0), 1);

    if (hit) {
        int steps = 128;
        
        float rayStep = abs (bt.y - bt.x) / float (steps);
        float rayT = bt.x;
        vec3  rayPos  = camera.position + normRayDir * rayT;
        
        for (int i = 0; i < steps; ++i) {
            float volume = texture (agySampler, rayPos).r;
            if (volume > 0.1f) {
                presented = vec4 (vec3 (volume), 1.f);
                break;
            }
        
            rayT += rayStep;
            rayPos = camera.position + normRayDir * rayT;
        }


    }

    presented = vec4 (presented.rgb * 1.2f, 1);

    //presented = vec4 (texture (agySampler, vec3 (uv.y, uv.x, fract(time.time*0.1f))).rrr, 1);
    
    copy[0] = vec4 (texture (agy2dSampler, uv).rgb, 1);
    copy[1] = result;
}