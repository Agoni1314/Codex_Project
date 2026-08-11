import assert from "node:assert/strict";
import test from "node:test";

async function render() {
  const workerUrl = new URL("../dist/server/index.js", import.meta.url);
  workerUrl.searchParams.set("test", `${process.pid}-${Date.now()}`);
  const { default: worker } = await import(workerUrl.href);

  return worker.fetch(
    new Request("http://localhost/", { headers: { accept: "text/html" } }),
    { ASSETS: { fetch: async () => new Response("Not found", { status: 404 }) } },
    { waitUntil() {}, passThroughOnException() {} },
  );
}

test("renders the complete pet grooming landing page", async () => {
  const response = await render();
  assert.equal(response.status, 200);

  const html = await response.text();
  assert.match(html, /<title>爪爪洗护/);
  assert.match(html, /洗得干净，.*也要玩得.*开心/s);
  assert.match(html, /基础净护/);
  assert.match(html, /安心可见/);
  assert.match(html, /店内环境/);
  assert.match(html, /接待与选品区/);
  assert.match(html, /到店指南/);
  assert.match(html, /成都太古里/);
  assert.match(html, /location-taikoo-li-real\.webp/);
  assert.match(html, /立即预约/);
  assert.doesNotMatch(html, /codex-preview|react-loading-skeleton/);
});
