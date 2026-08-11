import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "爪爪洗护｜温柔专业的宠物洗护店",
  description: "不催促、不强迫，给每一只毛孩子轻松自在的洗护体验。",
};

export default function RootLayout({ children }: Readonly<{ children: React.ReactNode }>) {
  return <html lang="zh-CN"><body>{children}</body></html>;
}
