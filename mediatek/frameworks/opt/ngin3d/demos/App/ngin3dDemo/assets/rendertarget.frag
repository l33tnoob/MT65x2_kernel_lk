/**************************************************************************
 *
 * Copyright (c) 2013 MediaTek Inc. All Rights Reserved.
 * --------------------
 * This software is protected by copyright and the information contained
 * herein is confidential. The software may not be copied and the information
 * contained herein may not be used or disclosed except with the written
 * permission of MediaTek Inc.
 *
 ***************************************************************************/
/*
 * A simple shader to demonstrate that a scene can be rendered to an image and
 * then that image can be rendered to the screen using a customized effect.
 */

// Using high precision floats will not give optimal performance, but it makes
// the shader code simpler for this example.
precision highp float;

uniform sampler2D u_m_diffuseTexture;

uniform sampler2D u_texture;

varying vec2 v_texCoord;
varying vec3 v_normal;

// The main fragment shader function.
void main()
{
  // Add a constant colour to the texture colour so that we render something
  // even where the texture is black.
  vec4 diffuse = texture2D(u_m_diffuseTexture, v_texCoord)
    + vec4( 0.4, 0.4, 0.4, 0.0);

  // Swap red and greed channel using swizzling for the front face only.
  if( gl_FrontFacing )
      diffuse = diffuse.grba;

  // Some simple lighting to show the orientation of the object.
  gl_FragColor = diffuse.yxzw * max( dot( v_normal, vec3( 0.0, 0.0, 1.0 ) ),
                                     dot( v_normal, vec3( 0.0, 0.0, -1.0 ) ) );
  gl_FragColor.a = 1.0;
}
