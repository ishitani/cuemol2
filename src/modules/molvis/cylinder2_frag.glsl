// -*-Mode: C++;-*-
//
//  fragment shader for cylinders
//

@include "lib_common.glsl"

////////////////////
// Uniform variables

uniform float frag_alpha;

uniform float u_edge;

// edge color
uniform vec4 u_edgecolor;

uniform bool u_bsilh;

////////////////////
// Varying variables

varying vec4 v_color;
varying vec2 v_impos;
varying vec4 v_ecpos;

varying float v_ndec;

// Direction flag of P21
// (sign of P21.x)
varying float v_dirflag;

varying float v_depmx;

// For normal calculation
varying vec2 v_normadj;
varying mat2 v_normrot;
varying vec2 v_vwdir;

void main()
{

  float adj_cen = sqrt(1.0 - v_impos.x*v_impos.x);
  float disp_cir = adj_cen * v_ndec;
  float depth = adj_cen * v_depmx;

  float imy = v_impos.y;
  imy -= disp_cir * v_dirflag;
  
  // discard the impostor pixels out of the cylinder
  if (imy <= -1.0 ||
      1.0 <= imy) {
    discard;
    return;
  }

  bool bEdge = false;
  if (v_impos.x<-1.0 ||
      v_impos.x> 1.0) {

    if (v_impos.y<-1.0 ||
	v_impos.y> 1.0) {
      discard;
      return;
    }  
    else {
      // edge line
      bEdge = true;
    }
  }

  /////
  // Calculate normal

  vec3 normal = vec3(v_impos.x, v_normadj.x * adj_cen, v_normadj.y * adj_cen);
  //mat2 normrot = mat2(v_vwdir.x, v_vwdir.y,-v_vwdir.y, v_vwdir.x);
  normal.xy *= v_normrot;

  /////
  // Change depth & output fragment color

  float far = gl_DepthRange.far;
  float near = gl_DepthRange.near;
  
  vec4 ecpos = v_ecpos;
  if (!bEdge)
    ecpos.z += depth;
  vec4 clip_space_pos = gl_ProjectionMatrix * ecpos;
  
  float ndc_depth = clip_space_pos.z / clip_space_pos.w;
  
  float fd = (((far-near) * ndc_depth) + near + far) / 2.0;
  
  // re-apply clipping by the view volume
  if (fd>far) {
    discard;
  }
  else if (fd<near) {
    discard;
    //normal = vec3(0.0, 0.0, 1.0);
    //fd = near;
  }
  else {
    // edge or body

    // set depth
    if (bEdge&&u_bsilh)
      gl_FragDepth = 0.99;
    else
      gl_FragDepth = fd;
    
    // color calculation
    vec4 color;
    if (bEdge) {
      color = vec4(u_edgecolor.rgb, v_color.a);
    }
    else {
      //vec4 color = v_color;
      color = flight(normal, ecpos, v_color);
    }
    
    // Fog calculation
    //gl_FragColor = calcFogAlpha(color, ffog(ecpos.z), frag_alpha);
    gl_FragColor = color;
  }

  /*
  if (v_impos.y<-1.0 ||
      v_impos.y> 1.0) {
    gl_FragColor = vec4(0,0,1,1);
  }  
  else {
    gl_FragColor = vec4((v_impos.x+1)*0.5,
                        (v_impos.y+1)*0.5, 0, 1);
  }
   */
  
  //gl_FragColor = v_color;
  //gl_FragColor = vec4(1,1,1,1);
}
