import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";

test("uses official Next.js for local development and keeps Sites deployment scripts", async () => {
  const pkg = JSON.parse(await readFile(new URL("../package.json", import.meta.url), "utf8"));
  const tsconfig = JSON.parse(await readFile(new URL("../tsconfig.json", import.meta.url), "utf8"));

  assert.equal(pkg.dependencies.next, "16.2.6");
  assert.equal(pkg.scripts.dev, "next dev");
  assert.equal(pkg.scripts.build, "next build");
  assert.equal(pkg.scripts.start, "next start");
  assert.equal(pkg.scripts["build:sites"], "vinext build");
  assert.equal(pkg.scripts["dev:sites"], "vinext dev");
  assert.ok(tsconfig.exclude.includes("worker"));
  assert.ok(tsconfig.exclude.includes("db"));
});
