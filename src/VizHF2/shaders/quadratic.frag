#version 450

layout (std140, binding = 0) uniform Time {
    float time;
} time;

layout (std140, binding = 1) uniform Camera {
    mat4 viewMatrix;
    mat4 VP;
    mat4 rayDirMatrix;
    vec3 position;
    vec3 viewDir;
    uint displayMode;
    float asd;
} camera;

struct Quadric {
    mat4 surface;
    mat4 clipper;
    vec4 kd;
    vec4 kr;
    vec4 ks;
    vec4 d2;
};

struct Light {
    vec4 position;
    vec4 powerDensity;
};

#define QUADRIC_COUNT 5
#define LIGHT_COUNT 2

layout (std140, binding = 2) uniform Quadrics {
    Quadric quadrics[QUADRIC_COUNT];
};


layout (std140, binding = 3) uniform Lights {
    Light lights[LIGHT_COUNT];
};


layout (location = 0) in vec2 uv;
layout (location = 1) in vec3 rayDirection;

layout (location = 0) out vec4 presented;


float IntersectClippedQuadric (const vec4 e, const vec4 d, const mat4 coeff, const mat4 clipper)
{
    const float a = dot (d * coeff, d);
    const float b = dot (e * coeff, d) + dot (d * coeff, e);
    const float c = dot (e * coeff, e);

    const float disc = b * b - 4.f * a * c;
    if (disc < 0.f)
      return -1.0;

    float t1 = (-b - sqrt (disc)) / (2.f * a);
    float t2 = (-b + sqrt (disc)) / (2.f * a);

    const vec4 h1 = e + d * t1;	
    const vec4 h2 = e + d * t2;

    if (dot (h1 * clipper, h1) > 0.0) t1 = -1.0;
    if (dot (h2 * clipper, h2) > 0.0) t2 = -1.0;	

    return (t1 < 0.f)
        ? t2
        : ((t2 < 0.f)
            ? t1
            : min (t1, t2));
}


struct Hit {
    float t;
    vec4  position;
    uint  quadricIndex;
};


Hit FindBestHit (vec4 e, vec4 d) {
    Hit bestHit;
    bestHit.t = 100000.0;
    bestHit.quadricIndex = 0;

    for (int i = 0; i < QUADRIC_COUNT; ++i) {
        const float t = IntersectClippedQuadric (e, d, quadrics[i].surface, quadrics[i].clipper);
        if (0.f < t && t < bestHit.t) {
            bestHit.t = t;
            bestHit.quadricIndex = i;
        }
    }

    if (bestHit.t > 100000.0 - 1) {
        bestHit.t = -1.0;
    } else {
        bestHit.position = e + d * bestHit.t;
    }

    return bestHit;
}


vec3 Shade (vec3 normal, vec3 viewDir, vec3 lightDir, vec3 powerDensity, vec3 kd, vec3 ks) 
{
    const vec3 halfway = normalize (viewDir + lightDir);
    if (dot (normal, lightDir) < 0.f) {
        return vec3 (0, 0, 0);
    }
    const float cosa = max (dot (normal, lightDir), 0.f);
    const float cosb = max (dot (normal, viewDir), 0.f);
    return 
        powerDensity * kd * cosa
        +
        powerDensity * ks * pow (max (dot (halfway, normal), 0), 20) * cosa / max (cosa, cosb);
        ;
}

#define EPS 1e-3

const bool showShadows = true;

vec3 DirectLighting (vec4 x, vec3 normal, vec3 viewDir, vec3 kd, vec3 ks)
{
    vec3 radiance = vec3 (0, 0, 0);
    for (int i = 0; i < LIGHT_COUNT; ++i) {
        vec3 lightDir = lights[i].position.xyz;
        vec3 powerDensity = lights[i].powerDensity.rgb;
    
        Hit shadowHit = FindBestHit (x + vec4 (normal, 0) * EPS, vec4 (lightDir, 0));
        if (!showShadows || shadowHit.t < 0.f) {
            radiance += Shade (normal, viewDir, lightDir, powerDensity, kd, ks);
        }
    }
    return radiance;
}


vec3 GetQuadricNormal (mat4 surface, vec4 hitPosition)
{
    return normalize ((hitPosition * surface + surface * hitPosition).xyz);
}


struct Ray {
    vec4 startPos;
    vec4 direction;
    vec3 reflectanceProduct;
    bool inside;
};


void main ()
{
    vec3 radiance = vec3 (0, 0, 0);
    
    // vec3 reflectanceProduct = vec3 (1, 1, 1);

    const vec3 background = vec3 (0.2);

    #define MAX_RAY_COUNT 64
    Ray rays[MAX_RAY_COUNT];
    int rayCount = 0;

    #define ADD_RAY(r) if (rayCount < MAX_RAY_COUNT) {rays[rayCount] = r; rayCount++;} 

    Ray initialRay;
    initialRay.startPos           = vec4 (camera.position, 1);
    initialRay.direction          = vec4 (normalize (rayDirection.xyz), 0);
    initialRay.reflectanceProduct = vec3 (1, 1, 1);
    initialRay.inside             = false;
    ADD_RAY (initialRay);

    const int maxStartedRays = 8;
    int startedRays = 0;

    while (rayCount > 0 && startedRays++ <= maxStartedRays) {
        const Ray currentRay = rays[rayCount - 1];
        rayCount--;

        const Hit hit = FindBestHit (currentRay.startPos, currentRay.direction);

        if (hit.t > 0.f) {
            vec3 normal = GetQuadricNormal (quadrics[hit.quadricIndex].surface, hit.position);
        
                if (dot (normal, currentRay.direction.xyz) > 0.f) {
                    normal = -normal;
                }
//
            radiance +=
                currentRay.reflectanceProduct * 
                DirectLighting (hit.position, normal, -currentRay.direction.xyz, quadrics[hit.quadricIndex].kd.rgb, quadrics[hit.quadricIndex].ks.rgb);

            {
                Ray reflectedRay;
                reflectedRay.startPos           = hit.position + vec4 (normal, 0) * EPS;
                reflectedRay.direction          = vec4 (reflect (currentRay.direction.xyz, normal), 0);
                reflectedRay.reflectanceProduct = currentRay.reflectanceProduct * quadrics[hit.quadricIndex].kr.rgb;
                reflectedRay.inside             = currentRay.inside;
                if (length (reflectedRay.reflectanceProduct) > EPS) {
                    ADD_RAY (reflectedRay);
                }
            }

            {
                Ray refractedRay;
                refractedRay.startPos           = hit.position - vec4 (normal, 0) * EPS;
                refractedRay.direction          = vec4 (refract (currentRay.direction.xyz, normal, 1.3333), 0);
                refractedRay.reflectanceProduct = currentRay.reflectanceProduct * quadrics[hit.quadricIndex].kr.rgb;
                refractedRay.inside             = !currentRay.inside;
                if (length (refractedRay.reflectanceProduct) > EPS) {
                    ADD_RAY (refractedRay);
                }
            }

        } else {
            radiance += currentRay.reflectanceProduct * background;
        }
    }

//
//    for (int i = 0; i < 6; ++i) {
//        Hit hit = FindBestHit (e, d);
//
//        if (hit.t > 0.f) {        
//            vec4 hitPosition = e + d * hit.t;            
//            vec3 normal = GetQuadricNormal (quadrics[hit.quadricIndex].surface, hitPosition);
//        
//            if (dot (normal, d.xyz) > 0.f) {
//                normal = -normal;
//            }
//
//            radiance +=
//                reflectanceProduct * 
//                DirectLighting (hitPosition, normal, -d.xyz, quadrics[hit.quadricIndex].kd.rgb, quadrics[hit.quadricIndex].ks.rgb);
//
//            reflectanceProduct *= quadrics[hit.quadricIndex].kr.rgb;
//
//            e = hitPosition + vec4 (normal, 0) * EPS;            
//            d.xyz = reflect (d.xyz, normal);
//
//        } else {
//            radiance += reflectanceProduct * background;            
//            break;
//        }
//    }

    presented = vec4 (radiance, 1);
}