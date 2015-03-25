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

// Using high precision floats will not give optimal performance, but it makes
// the shader code simpler for this example.
precision highp float;

// Uniforms correspond to "properties" set in the "wave.mat" file, and linked
// to the shader in the "wave.sp" file.
uniform sampler2D u_m_diffuseTexture;
uniform vec4 u_m_diffuseColour;

uniform sampler2D u_texture;
uniform vec3 u_touchPosition;
uniform bool u_invert;

// Varyings allow values to be passed between vertex and fragment shaders.  The
// values are interpolated to produce values for each fragment.
varying vec2 v_texCoord;
varying vec2 v_texCoord2;

// The main fragment shader function.
void main()
{
  // Multiply the colour and texture sample to get the final fragment colour.
  vec4 diffuse = u_m_diffuseColour * texture2D(u_m_diffuseTexture, v_texCoord);

  // Modulate the blending of the onion texture onto the model by the
  // brightness of the original colour (this just makes the model look slightly
  // less busy).
  float brightness = length(diffuse.rgb);
  diffuse *= texture2D(u_texture, v_texCoord2) * brightness +
    vec4(1.0 - brightness);

  // Calculate the how much of the inversion effect shows, depending on the
  // distance of the fragment from the touch position (gives a circular effect).
  float fade = clamp(
      250.0 / length(gl_FragCoord.xy - u_touchPosition.xy) - 0.7, 0.0, 1.0);

  // Invert the "inversion" effect if the flag is set.
  if (u_invert)
  {
    fade = 1.0 - fade;
  }

  // Blend together the inverted and non-inverted colours.
  gl_FragColor.rgb = clamp(mix(vec3(1.0) - diffuse.rgb, diffuse.rgb, fade), 0.0, 1.0);
  gl_FragColor.a = 1.0;
}
