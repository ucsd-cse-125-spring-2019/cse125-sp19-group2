#version 410 core

in vec3 pass_fragPos;
in vec3 pass_normal;
in vec2 pass_uv;

struct Material
{
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};

struct PointLight
{
  vec3 position;
  
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  
  float constant;
  float linear;
  float quadratic;
};

// Lighting
uniform PointLight u_light;
uniform Material   u_objMat;
uniform vec3       u_viewPos;

// Output
out vec4 out_color;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  // Ambient
  vec3 ambient = light.ambient * texture(u_objMat.diffuse, pass_uv).rgb;
  
  // Diffuse
  vec3 lightDir = normalize(light.position - fragPos);
  float diffAmt = max(dot(normal, lightDir), 0);
  vec3 diffuse = light.diffuse * diffAmt * texture(u_objMat.diffuse, pass_uv).rgb;
  
  // Specular
  vec3 reflectDir = reflect(-lightDir, normal);
  float specAmt = pow(max(dot(viewDir, reflectDir), 0), u_objMat.shininess);
  vec3 specular = light.specular * specAmt * texture(u_objMat.specular, pass_uv).rgb;
  
  // Attenuation
  float dist = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant +
                             (light.linear * dist) +
                             (light.quadratic * (dist * dist)));
  
  ambient  *= attenuation;
  diffuse  *= attenuation;
  specular *= attenuation;
  
  return ambient + diffuse + specular;
}

void main(void)
{
  vec3 normal = normalize(pass_normal);
  vec3 viewDir = normalize(u_viewPos - pass_fragPos);
  
  vec3 resultCol = CalcPointLight(u_light, normal, pass_fragPos, viewDir);
  
  out_color = vec4(resultCol, 1.0f);
}
