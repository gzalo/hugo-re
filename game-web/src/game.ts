/**
 * Port of game/game.py.
 * Main game loop: 4 × OffscreenCanvas (320×240) → composite Canvas (640×480)
 * → WebGL2 post-processing shader.
 */

import { Config } from './config';
import { globalState } from './globalState';
import { PostProcessing } from './postProcessing';
import { Tween } from './tween';
import { Splat } from './effects/splat';
import { PhoneEvents } from './phoneEvents';
import { TvShowParent } from './tvShow/tvShowParent';
import { loadSurfaceRes } from './resource';
import type { GameData } from './gameData';

// Composite positions for the 4 quadrants
const POSITIONS: [number, number][] = [
  [0, 0],
  [320, 0],
  [0, 240],
  [320, 240],
];

// Orb animation entry: [sourceCountry, targetCountry, startTime]
type EffectiveAttack = [string, string, number];

export class Game {
  private gl: WebGL2RenderingContext;
  private program: WebGLProgram;
  private vao: WebGLVertexArrayObject;
  private frameTex: WebGLTexture;

  private compositeCanvas: OffscreenCanvas;
  private compositeCtx: OffscreenCanvasRenderingContext2D;

  private screens: OffscreenCanvas[];
  private screenCtxs: OffscreenCanvasRenderingContext2D[];

  private tvShows: TvShowParent[];
  private posByCountry: Record<string, [number, number]>;
  readonly postProcessing: PostProcessing;

  private startTime: number;
  private lastFrameTime = 0;
  private readonly TARGET_FRAME_MS = 1000 / 30;  // 30fps

  // Pending keyboard events (cleared each frame)
  private pendingKeys: string[] = [];
  private readonly keydownHandler: (e: KeyboardEvent) => void;

  // Pending phone button events from UI clicks (cleared each frame)
  private pendingPhonePickup = [false, false, false, false];
  private pendingPhoneHangup  = [false, false, false, false];

  // Broadcast events from the common phone panel (all players)
  private pendingBroadcastPickup = false;
  private pendingBroadcastHangup  = false;
  private pendingBroadcastKeys: string[] = [];

  // Effective attacks for orb rendering
  private effectiveAttacks: EffectiveAttack[] = [];

  // UI images
  private logo: ImageBitmap | null = null;
  private phoneIcons: ImageBitmap[] = [];
  private phoneIconsActive: ImageBitmap[] = [];

  constructor(
    canvas: HTMLCanvasElement,
    tvShowContexts: GameData[],
    vertShader: string,
    fragShader: string,
  ) {
    // WebGL2 setup
    const gl = canvas.getContext('webgl2');
    if (!gl) throw new Error('WebGL2 not supported');
    this.gl = gl;

    this.program = this.buildProgram(vertShader, fragShader);
    this.vao = this.buildVao();
    this.frameTex = this.buildTexture();

    // Composite canvas (640×480, 2D)
    this.compositeCanvas = new OffscreenCanvas(640, 480);
    this.compositeCtx = this.compositeCanvas.getContext('2d')!;

    // Per-channel offscreen canvases (320×240)
    this.screens = POSITIONS.map(() => new OffscreenCanvas(320, 240));
    this.screenCtxs = this.screens.map(s => s.getContext('2d')!);

    // TV show instances
    this.tvShows = tvShowContexts.map(ctx => new TvShowParent(ctx));
    this.posByCountry = {};
    for (let i = 0; i < this.tvShows.length; i++) {
      this.posByCountry[this.tvShows[i].country] = POSITIONS[i];
    }

    this.postProcessing = new PostProcessing();
    this.startTime = performance.now() / 1000;

    // Keyboard handler
    this.keydownHandler = (e: KeyboardEvent) => {
      this.pendingKeys.push(e.key);
    };
    document.addEventListener('keydown', this.keydownHandler);
  }

  async loadUiAssets(): Promise<void> {
    [
      this.logo,
      ...this.phoneIcons
    ] = await Promise.all([
      loadSurfaceRes('images/logo.png'),
      loadSurfaceRes('images/phone0_small.png'),
      loadSurfaceRes('images/phone1_small.png'),
      loadSurfaceRes('images/phone2_small.png'),
      loadSurfaceRes('images/phone3_small.png'),
    ]);

    this.phoneIconsActive = await Promise.all([
      loadSurfaceRes('images/phone0_small_active.png'),
      loadSurfaceRes('images/phone1_small_active.png'),
      loadSurfaceRes('images/phone2_small_active.png'),
      loadSurfaceRes('images/phone3_small_active.png'),
    ]);
  }

  /** Trigger an off-hook (pick up phone) event for player i from the UI. */
  setOffhook(i: number): void { if (i >= 0 && i < 4) this.pendingPhonePickup[i] = true; }

  /** Trigger a hung-up (hang up phone) event for player i from the UI. */
  setHungup(i: number): void { if (i >= 0 && i < 4) this.pendingPhoneHangup[i] = true; }

  /** Broadcast off-hook to all players from the common phone panel. */
  setAllOffhook(): void { this.pendingBroadcastPickup = true; }

  /** Broadcast hung-up to all players from the common phone panel. */
  setAllHungup(): void { this.pendingBroadcastHangup = true; }

  /** Broadcast a digit key ('0'–'9', '*', '#') to all players. */
  pressBroadcastKey(key: string): void { this.pendingBroadcastKeys.push(key); }

  /** Start the game loop. */
  start(): void {
    requestAnimationFrame(this.loop.bind(this));
  }

  private loop(timestamp: number): void {
    const elapsed = timestamp - this.lastFrameTime;
    if (elapsed < this.TARGET_FRAME_MS) {
      requestAnimationFrame(this.loop.bind(this));
      return;
    }
    this.lastFrameTime = timestamp - (elapsed % this.TARGET_FRAME_MS);

    // Update global frame time
    globalState.frameTime = timestamp / 1000;

    // Build per-player events from pending keys
    const phoneEvents = POSITIONS.map(() => new PhoneEvents());
    for (const key of this.pendingKeys) {
      if (key === Config.BTN_EXIT) {
        this.stop();
        return;
      }
      for (let i = 0; i < 4; i++) {
        if (key === Config.BTN_OFF_HOOK[i]) phoneEvents[i].offhook = true;
        if (key === Config.BTN_HUNG_UP[i])  phoneEvents[i].hungup  = true;
        if (key === Config.BTN_0[i])        phoneEvents[i].press_0 = true;
        if (key === Config.BTN_1[i])        phoneEvents[i].press_1 = true;
        if (key === Config.BTN_2[i])        phoneEvents[i].press_2 = true;
        if (key === Config.BTN_3[i])        phoneEvents[i].press_3 = true;
        if (key === Config.BTN_4[i])        phoneEvents[i].press_4 = true;
        if (key === Config.BTN_5[i])        phoneEvents[i].press_5 = true;
        if (key === Config.BTN_6[i])        phoneEvents[i].press_6 = true;
        if (key === Config.BTN_7[i])        phoneEvents[i].press_7 = true;
        if (key === Config.BTN_8[i])        phoneEvents[i].press_8 = true;
        if (key === Config.BTN_9[i])        phoneEvents[i].press_9 = true;
      }
    }
    this.pendingKeys = [];

    // Apply pending UI phone-button events
    for (let i = 0; i < 4; i++) {
      if (this.pendingPhonePickup[i]) { phoneEvents[i].offhook = true; this.pendingPhonePickup[i] = false; }
      if (this.pendingPhoneHangup[i])  { phoneEvents[i].hungup  = true; this.pendingPhoneHangup[i]  = false; }
    }

    // Apply broadcast events from the common phone panel (all players)
    const broadcastKeys = this.pendingBroadcastKeys.splice(0);
    if (this.pendingBroadcastPickup || this.pendingBroadcastHangup || broadcastKeys.length) {
      const digitMap: Partial<Record<string, Exclude<keyof PhoneEvents, 'anySet'>>> = {
        '0': 'press_0', '1': 'press_1', '2': 'press_2', '3': 'press_3',
        '4': 'press_4', '5': 'press_5', '6': 'press_6', '7': 'press_7',
        '8': 'press_8', '9': 'press_9', '*': 'press_star', '#': 'press_pound',
      };
      for (let i = 0; i < 4; i++) {
        if (this.pendingBroadcastPickup) phoneEvents[i].offhook = true;
        if (this.pendingBroadcastHangup)  phoneEvents[i].hungup  = true;
        for (const k of broadcastKeys) {
          const prop = digitMap[k];
          if (prop) phoneEvents[i][prop] = true;
        }
      }
      this.pendingBroadcastPickup = false;
      this.pendingBroadcastHangup  = false;
    }

    // Update anyPlaying
    globalState.anyPlaying = this.tvShows.some(tv => tv.isPlaying());

    // Handle events + render each channel
    for (let i = 0; i < 4; i++) {
      this.tvShows[i].handleEvents(phoneEvents[i]);
      this.screenCtxs[i].clearRect(0, 0, 320, 240);
      this.tvShows[i].render(this.screenCtxs[i]);
    }

    // Handle gamepad (post-processing controls)
    this.postProcessing.handleEvents();

    // Process cross-player attacks
    const attacks = [...globalState.attacks];
    globalState.attacks = [];
    for (const attack of attacks) {
      const [currentCountry, effect, attackStartTime] = attack;
      const validShows = this.tvShows.filter(
        tv => tv.country !== currentCountry && tv.isPlaying(),
      );
      if (validShows.length === 0) continue;
      const target = validShows[Math.floor(Math.random() * validShows.length)];
      target.externalEffect(effect);
      this.effectiveAttacks.push([currentCountry, target.country, attackStartTime]);
    }

    // Composite 4 screens onto the main canvas
    const cctx = this.compositeCtx;
    cctx.clearRect(0, 0, 640, 480);
    for (let i = 0; i < 4; i++) {
      cctx.drawImage(this.screens[i], POSITIONS[i][0], POSITIONS[i][1]);
    }

    // Overlays
    if (!globalState.anyPlaying) {
      if (this.logo) cctx.drawImage(this.logo, 0, 0);
    } else {
      for (let i = 0; i < 4; i++) {
        const icon = phoneEvents[i].anySet()
          ? this.phoneIconsActive[i]
          : this.phoneIcons[i];
        if (icon) cctx.drawImage(icon, POSITIONS[i][0], POSITIONS[i][1]);
      }

      // Orb animation
      const now = globalState.frameTime;
      this.effectiveAttacks = this.effectiveAttacks.filter(([src, dst, t]) => {
        const dt = now - t;
        if (dt > Config.EFFECT_DURATION_ORB) return false;
        const [sx, sy] = this.posByCountry[src];
        const [dx, dy] = this.posByCountry[dst];
        const ox = Tween.mapEaseIn(dt, 0, Config.EFFECT_DURATION_ORB, sx + 70, dx + 70);
        const oy = Tween.mapEaseIn(dt, 0, Config.EFFECT_DURATION_ORB, sy + 70, dy + 70);
        if (Splat.orb) cctx.drawImage(Splat.orb, ox, oy);
        return true;
      });
    }

    // Upload composite canvas to WebGL texture and render
    this.renderFrame();

    requestAnimationFrame(this.loop.bind(this));
  }

  private renderFrame(): void {
    const gl = this.gl;
    const time = performance.now() / 1000 - this.startTime;

    gl.bindTexture(gl.TEXTURE_2D, this.frameTex);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.compositeCanvas);

    gl.useProgram(this.program);
    gl.uniform1i(gl.getUniformLocation(this.program, 'tex'), 0);
    this.postProcessing.apply(gl, this.program, time, globalState.anyPlaying);

    gl.bindVertexArray(this.vao);
    gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
  }

  stop(): void {
    document.removeEventListener('keydown', this.keydownHandler);
    for (const tv of this.tvShows) tv.cleanup();
  }

  // ─── WebGL2 helpers ───────────────────────────────────────────────────────

  private buildProgram(vert: string, frag: string): WebGLProgram {
    const gl = this.gl;

    const vs = gl.createShader(gl.VERTEX_SHADER)!;
    gl.shaderSource(vs, vert);
    gl.compileShader(vs);
    if (!gl.getShaderParameter(vs, gl.COMPILE_STATUS)) {
      throw new Error('Vertex shader error: ' + gl.getShaderInfoLog(vs));
    }

    const fs = gl.createShader(gl.FRAGMENT_SHADER)!;
    gl.shaderSource(fs, frag);
    gl.compileShader(fs);
    if (!gl.getShaderParameter(fs, gl.COMPILE_STATUS)) {
      throw new Error('Fragment shader error: ' + gl.getShaderInfoLog(fs));
    }

    const prog = gl.createProgram()!;
    gl.attachShader(prog, vs);
    gl.attachShader(prog, fs);
    gl.linkProgram(prog);
    if (!gl.getProgramParameter(prog, gl.LINK_STATUS)) {
      throw new Error('Shader link error: ' + gl.getProgramInfoLog(prog));
    }
    return prog;
  }

  private buildVao(): WebGLVertexArrayObject {
    const gl = this.gl;

    // Triangle strip: 4 vertices with (x, y, u, v) each
    const vertices = new Float32Array([
      -1,  1,  0, 0,
       1,  1,  1, 0,
      -1, -1,  0, 1,
       1, -1,  1, 1,
    ]);

    const vbo = gl.createBuffer()!;
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
    gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

    const vao = gl.createVertexArray()!;
    gl.bindVertexArray(vao);

    const vertLoc = gl.getAttribLocation(this.program, 'vert');
    gl.enableVertexAttribArray(vertLoc);
    gl.vertexAttribPointer(vertLoc, 2, gl.FLOAT, false, 16, 0);

    const texLoc = gl.getAttribLocation(this.program, 'texcoord');
    gl.enableVertexAttribArray(texLoc);
    gl.vertexAttribPointer(texLoc, 2, gl.FLOAT, false, 16, 8);

    gl.bindVertexArray(null);
    return vao;
  }

  private buildTexture(): WebGLTexture {
    const gl = this.gl;
    const tex = gl.createTexture()!;
    gl.bindTexture(gl.TEXTURE_2D, tex);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
    return tex;
  }
}
