# WebAssembly Plorth [![npm][npm-image]][npm-url]

[npm-image]: https://img.shields.io/npm/v/plorth.svg
[npm-url]: https://npmjs.org/package/plorth

[Plorth] interpreter as WebAssembly module, compiled with [Emscripten].

## Quick usage example

```JavaScript
import Plorth from 'plorth';

const interpreter = new Plorth();
interpreter.execute('1 1 + println');
```

[Plorth]: https://github.com/RauliL/plorth
[Emscripten]: https://kripken.github.io/emscripten-site/
