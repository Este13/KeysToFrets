import { build } from 'esbuild';
import { fileURLToPath } from 'node:url';
import path from 'node:path';

const here = path.dirname(fileURLToPath(import.meta.url));
const entry = path.join(here, 'tonal-entry.js');
const outfile = path.join(here, '..', 'ui', 'vendor', 'tonal.js');

await build({
  entryPoints: [entry],
  outfile,
  bundle: true,
  minify: true,
  format: 'iife',
  target: 'chrome110',
  legalComments: 'inline'
});
console.log('written', outfile);
