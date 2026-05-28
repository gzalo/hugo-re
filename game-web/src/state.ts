import { globalState } from './globalState';
import type { GameData } from './gameData';
import type { PhoneEvents } from './phoneEvents';

export type StateClass = new (context: GameData) => State;

export class State {
  startTime: number = 0;
  events: Record<string, number> = {};
  context: GameData;

  constructor(context: GameData) {
    this.context = context;
  }

  processEvents(_phoneEvents: PhoneEvents): StateClass | null {
    return null;
  }

  render(_ctx: OffscreenCanvasRenderingContext2D): void {}

  onEnter(): void {
    this.startTime = globalState.frameTime;
  }

  onExit(): void {}

  getStateTime(): number {
    return globalState.frameTime - this.startTime;
  }

  getFrameIndex(): number {
    return Math.floor(this.getStateTime() * 10);
  }

  getFrameIndexFast(): number {
    return Math.floor(this.getStateTime() * 20);
  }

  oneShot(delta: number, name: string): boolean {
    if (!(name in this.events) && globalState.frameTime - this.startTime > delta) {
      this.events[name] = globalState.frameTime - this.startTime;
      return true;
    }
    return false;
  }

  every(delta: number, name: string, offset: number = 0.0): boolean {
    if (!(name in this.events)) {
      if (globalState.frameTime - this.startTime > offset) {
        this.events[name] = globalState.frameTime + delta;
        return true;
      }
    } else if (globalState.frameTime - this.events[name] >= 0) {
      this.events[name] = globalState.frameTime + delta;
      return true;
    }
    return false;
  }
}
