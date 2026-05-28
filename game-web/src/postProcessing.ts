/**
 * Port of game/post_processing.py.
 * Manages WebGL2 post-processing shader uniforms.
 * Joystick handled via Gamepad API (replaces pygame joystick).
 */

import { Config } from './config';
import { Tween } from './tween';

export class PostProcessing {
  wavyness = 0;
  bitcrush = 256;
  hueShift = 0;
  scale = 1;
  lineGlitch = 0;
  chromaticAberration = 0;

  /** When true, slider values are used instead of the gamepad. */
  slidersActive = false;

  handleEvents(): void {
    // Sliders take priority — skip gamepad when active
    if (this.slidersActive) return;

    const gamepads = navigator.getGamepads();
    const joy = gamepads[0];
    if (!joy) return;
    if (joy.axes.length < 6) return;

    this.scale                  = Tween.map(joy.axes[0], -1, 1, 1, 4);
    Config.FOREST_BG_SPEED_MULTIPLIER = Tween.map(joy.axes[1], -1, 1, 1, 5);
    this.wavyness               = Tween.map(joy.axes[2], -1, 1, 0, 0.1);
    this.hueShift               = Tween.map(joy.axes[3], -1, 1, 0, 1);
    this.bitcrush               = Tween.mapBitcrush(joy.axes[4], -1, 1, 2, 256);
    this.lineGlitch             = Tween.map(joy.axes[4], -1, 1, 0, 0.5);
    this.chromaticAberration    = Tween.map(joy.axes[5], -1, 1, 0, 1);
  }

  apply(gl: WebGL2RenderingContext, program: WebGLProgram, time: number, anyPlaying: boolean): void {
    const u = (name: string) => gl.getUniformLocation(program, name);

    gl.uniform1f(u('time'), time);

    if (anyPlaying) {
      gl.uniform1f(u('wavyness'),            this.wavyness);
      gl.uniform1f(u('bitcrush'),            this.bitcrush);
      gl.uniform1f(u('hue_shift'),           this.hueShift);
      gl.uniform1f(u('scale'),               this.scale);
      gl.uniform1f(u('chromatic_aberration'), this.chromaticAberration);
      gl.uniform1f(u('line_glitch'),         this.lineGlitch);
    } else {
      gl.uniform1f(u('wavyness'),            0.01);
      gl.uniform1f(u('bitcrush'),            256);
      gl.uniform1f(u('hue_shift'),           0);
      gl.uniform1f(u('scale'),               1);
      gl.uniform1f(u('chromatic_aberration'), 0);
      gl.uniform1f(u('line_glitch'),         0);
    }
  }
}
