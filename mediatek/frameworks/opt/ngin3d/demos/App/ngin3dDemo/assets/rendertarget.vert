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

uniform mat4 u_t_modelViewProjection;
uniform mat3 u_t_normal;


attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec2 a_uv0;

varying vec2 v_texCoord;
varying vec3 v_normal;

// The main vertex shader function.
void main()
{
  // Just pass the model's UV coordinates straight through.
  v_texCoord = a_uv0;
  v_normal = u_t_normal * a_normal;

  gl_Position = u_t_modelViewProjection * a_position;

}
