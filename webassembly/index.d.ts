export default class Plorth {
  /** Version number of the Plorth interpreter. */
  static VERSION: string;

  /**
   * Compiles and executes given source code.
   */
  execute(sourceCode: string): void;

  /**
   * Returns the number of values in the stack.
   */
  depth(): number;

  /**
   * Extracts source code representation of value from the stack, based on
   * it's index on the stack.
   */
  stack(index: number): string;
}
