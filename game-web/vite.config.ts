import { defineConfig } from 'vite';

export default defineConfig({
  root: '.',
  base: '/hugo-alpha/',
  publicDir: '../game/resources',
  build: {
    outDir: 'dist',
    target: 'es2022',
  },
  server: {
    headers: {
      // Required for SharedArrayBuffer (not used here, but good practice)
      'Cross-Origin-Opener-Policy': 'same-origin',
      'Cross-Origin-Embedder-Policy': 'require-corp',
    },
  },
});
