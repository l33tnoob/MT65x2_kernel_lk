/**************************************************************************
 *
 * Copyright (c) 2012 MediaTek Inc. All Rights Reserved.
 * --------------------
 * This software is protected by copyright and the information contained
 * herein is confidential. The software may not be copied and the information
 * contained herein may not be used or disclosed except with the written
 * permission of MediaTek Inc.
 *
 ***************************************************************************/
/*
 * A pointless shader to showing an example of what can potentially be done
 * using custom materials in ngin3d.
 */

// Uniforms correspond to "properties" set in the "wave.mat" file, and linked
// to the shader in the "wave.sp" file.
uniform mat4 u_t_modelViewProjection;
uniform float u_time;

uniform int u_multiplier;
uniform float u_aspectRatio;

attribute vec4 a_position;
attribute vec2 a_uv0;

// Varyings allow values to be passed between vertex and fragment shaders.  The
// values are interpolated to produce values for each fragment.
varying vec2 v_texCoord;
varying vec2 v_texCoord2;

// The main vertex shader function.
void main()
{
  // Just pass the model's UV coordinates straight through.
  v_texCoord = a_uv0;

  // Calculate the screen-space vertex coordinates.
  vec4 position = u_t_modelViewProjection * a_position;

  // Offset the screen-space x-position of each vertex, using a sine wave which
  // varies with time and screen-space y-position.
  float offset = sin(u_time * float(u_multiplier) + position.y / 100.0) * 50.0;
  position.x += offset;

  // The second set of UV coordinates, used to map the onion texture to the
  // model, are calculated using the screen-space vertex positions, and drift
  // across the texture over time.
  v_texCoord2 = position.xy / position.w;
  v_texCoord2.x *= u_aspectRatio;
  v_texCoord2 += u_time / 7.0;

  gl_Position = position;
}
