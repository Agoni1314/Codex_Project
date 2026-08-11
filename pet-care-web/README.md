# 爪爪洗护

宠物洗护店单页面，使用官方 Next.js 16、React 19、TypeScript 和 App Router。

## 本地开发

要求 Node.js `>=22.13.0`。

```bash
npm install
npm run dev
```

打开 `http://localhost:3000`。

## 常用命令

- `npm run dev`：使用官方 Next.js 开发服务器
- `npm run build`：执行官方 Next.js 生产构建
- `npm run start`：启动官方 Next.js 生产服务器
- `npm test`：检查 Next.js 与 Sites 双运行配置
- `npm run dev:sites`：使用 Sites 适配器进行本地开发
- `npm run build:sites`：生成 Sites 发布产物
- `npm run test:sites`：构建并验证 Sites 服务端渲染

页面代码位于 `app/`，静态图片位于 `public/`。`vite.config.ts`、`worker/`、`build/` 和 `.openai/hosting.json` 仅用于保留现有 Sites 发布能力，不影响日常 Next.js 开发。
