/**
 * Entry point — mirrors game/__main__.py.
 * Shows a "Pick data folder" UI, initialises all resources, then starts the
 * game loop on the existing <canvas> element.
 */

import { setDataDir } from './resource';
import { CaveResources } from './cave/caveResources';
import { ForestResources } from './forest/forestResources';
import { ScoreboardResources } from './scoreboard/scoreboardResources';
import { TvShowResources } from './tvShow/tvShowResources';
import { Splat } from './effects/splat';
import { Game } from './game';
import { GameData } from './gameData';
import { Config } from './config';
import type { PostProcessing } from './postProcessing';

// ─── DOM refs ─────────────────────────────────────────────────────────────────

const launch     = document.getElementById('launch')!;
const pickBtn    = document.getElementById('pickFolder') as HTMLButtonElement;
const statusEl   = document.getElementById('status')!;
const gameArea   = document.getElementById('gameArea') as HTMLDivElement;
const gameCanvas = document.getElementById('gameCanvas') as HTMLCanvasElement;
const fxToggle   = document.getElementById('fxToggle') as HTMLButtonElement;
const fxPanel    = document.getElementById('fxPanel') as HTMLDivElement;
const fxReset    = document.getElementById('fxReset') as HTMLButtonElement;

// ─── Status helper ────────────────────────────────────────────────────────────

function setStatus(msg: string): void {
  statusEl.textContent = msg;
}

// ─── Shader loading ───────────────────────────────────────────────────────────

async function loadShaders(): Promise<[string, string]> {
  const base = import.meta.env.BASE_URL;
  const [vertResp, fragResp] = await Promise.all([
    fetch(`${base}shaders/main.vert`),
    fetch(`${base}shaders/main.frag`),
  ]);
  return [await vertResp.text(), await fragResp.text()];
}

// ─── FX panel wiring ─────────────────────────────────────────────────────────

type SliderDef = {
  id: string;
  prop: keyof PostProcessing;
  decimals: number;
};

const SLIDER_DEFS: SliderDef[] = [
  { id: 'sl-wavyness',            prop: 'wavyness',            decimals: 3 },
  { id: 'sl-bitcrush',            prop: 'bitcrush',            decimals: 0 },
  { id: 'sl-hueShift',            prop: 'hueShift',            decimals: 2 },
  { id: 'sl-scale',               prop: 'scale',               decimals: 2 },
  { id: 'sl-lineGlitch',          prop: 'lineGlitch',          decimals: 3 },
  { id: 'sl-chromaticAberration', prop: 'chromaticAberration', decimals: 2 },
];

function wireSliders(pp: PostProcessing): void {
  // Toggle panel open/close
  fxToggle.addEventListener('click', () => {
    const open = fxPanel.style.display === 'block';
    fxPanel.style.display = open ? 'none' : 'block';
    fxToggle.textContent = open ? 'FX ▾' : 'FX ▴';
  });

  // Reset button — hand control back to gamepad
  fxReset.addEventListener('click', () => {
    pp.slidersActive = false;
    for (const def of SLIDER_DEFS) {
      const input = document.getElementById(def.id) as HTMLInputElement;
      const valEl = document.getElementById('val-' + def.id.replace('sl-', ''))!;
      const v = pp[def.prop] as number;
      input.value = String(v);
      valEl.textContent = v.toFixed(def.decimals);
    }
  });

  // Wire each slider
  for (const def of SLIDER_DEFS) {
    const input  = document.getElementById(def.id) as HTMLInputElement;
    const valEl  = document.getElementById('val-' + def.id.replace('sl-', ''))!;

    // Sync display with initial value
    const initial = pp[def.prop] as number;
    input.value = String(initial);
    valEl.textContent = initial.toFixed(def.decimals);

    input.addEventListener('input', () => {
      const v = parseFloat(input.value);
      (pp as any)[def.prop] = v;
      pp.slidersActive = true;
      valEl.textContent = v.toFixed(def.decimals);
    });
  }
}

// ─── Main init ────────────────────────────────────────────────────────────────

async function init(dirHandle: FileSystemDirectoryHandle): Promise<void> {
  setStatus('Setting up resources…');
  setDataDir(dirHandle);

  setStatus('Loading shaders…');
  const [vertShader, fragShader] = await loadShaders();

  setStatus('Loading game resources…');
  await Promise.all([
    CaveResources.init(),
    ForestResources.init(),
    ScoreboardResources.init(),
    TvShowResources.init(),
    Splat.init(),
  ]);

  setStatus('Starting game…');

  // Create a GameData per country/channel
  const contexts = Config.COUNTRIES.map(country =>
    new GameData(country, 0, 0, 0, 0, [], [], [], false, 0, 0),
  );

  // Switch to game view, hide launcher UI
  launch.style.display = 'none';
  gameArea.style.display = 'flex';
  fxToggle.style.display = 'block';

  const game = new Game(gameCanvas, contexts, vertShader, fragShader);
  await game.loadUiAssets();

  wireSliders(game.postProcessing);

  // Wire per-player phone tube buttons
  document.querySelectorAll<HTMLButtonElement>('.phone-pickup').forEach(btn => {
    btn.addEventListener('click', () => {
      game.setOffhook(parseInt(btn.dataset.player ?? '0', 10));
    });
  });
  document.querySelectorAll<HTMLButtonElement>('.phone-hangup').forEach(btn => {
    btn.addEventListener('click', () => {
      game.setHungup(parseInt(btn.dataset.player ?? '0', 10));
    });
  });

  // Wire common phone panel (broadcasts to all players)
  document.getElementById('allPickup')!.addEventListener('click', () => game.setAllOffhook());
  document.getElementById('allHangup')!.addEventListener('click', () => game.setAllHungup());
  document.querySelectorAll<HTMLButtonElement>('.cp-digit').forEach(btn => {
    btn.addEventListener('click', () => game.pressBroadcastKey(btn.dataset.key!));
  });

  game.start();
}

// ─── Folder picker button ─────────────────────────────────────────────────────

pickBtn.addEventListener('click', async () => {
  try {
    const dirHandle = await (window as any).showDirectoryPicker({ mode: 'read' });
    pickBtn.disabled = true;
    await init(dirHandle);
  } catch (err: any) {
    if (err?.name !== 'AbortError') {
      setStatus(`Error: ${err?.message ?? err}`);
      pickBtn.disabled = false;
    }
  }
});
