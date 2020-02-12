const browserSync = require('browser-sync');

const bs = browserSync.create();

bs.watch('public/**/*.html', bs.reload);
bs.watch('public/**/*.js', bs.reload);
bs.watch('public/**/*.wasm', bs.reload);

bs.init({
  server: './public',
});
