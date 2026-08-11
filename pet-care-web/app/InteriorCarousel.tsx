"use client";

import { useEffect, useRef, useState } from "react";

const slides = [
  {
    src: "/interior-reception.webp",
    title: "接待与选品区",
    copy: "明亮开放的第一站，让等待也变得舒服。",
    alt: "奶油白、墨绿色与暖橙色搭配的宠物洗护店接待区",
  },
  {
    src: "/interior-washing.webp",
    title: "独立洗护区",
    copy: "分区操作、一宠一消毒，清洁过程更安心。",
    alt: "带有独立浴缸和透明隔板的宠物洗护区域",
  },
  {
    src: "/interior-styling.webp",
    title: "美容与安静休息区",
    copy: "完成造型后，在柔软安静的小窝里放松片刻。",
    alt: "设有美容台、柔光镜和宠物休息窝的美容区域",
  },
];

export function InteriorCarousel() {
  const [active, setActive] = useState(0);
  const [paused, setPaused] = useState(false);
  const pointerStart = useRef<number | null>(null);

  useEffect(() => {
    if (paused || window.matchMedia("(prefers-reduced-motion: reduce)").matches) return;
    const timer = window.setInterval(() => setActive((current) => (current + 1) % slides.length), 5200);
    return () => window.clearInterval(timer);
  }, [paused]);

  const move = (direction: number) => {
    setActive((current) => (current + direction + slides.length) % slides.length);
  };

  return (
    <div
      className="interior-carousel"
      aria-roledescription="轮播图"
      aria-label="店内不同区域"
      onMouseEnter={() => setPaused(true)}
      onMouseLeave={() => setPaused(false)}
      onFocus={() => setPaused(true)}
      onBlur={() => setPaused(false)}
      onPointerDown={(event) => { pointerStart.current = event.clientX; }}
      onPointerUp={(event) => {
        if (pointerStart.current === null) return;
        const distance = event.clientX - pointerStart.current;
        if (Math.abs(distance) > 45) move(distance > 0 ? -1 : 1);
        pointerStart.current = null;
      }}
    >
      <div className="interior-track" style={{ transform: `translateX(-${active * 100}%)` }}>
        {slides.map((slide, index) => (
          <figure className="interior-slide" key={slide.title} aria-hidden={active !== index}>
            <img src={slide.src} alt={slide.alt} loading={index === 0 ? "eager" : "lazy"} />
            <figcaption>
              <small>0{index + 1} / 03</small>
              <div><h3>{slide.title}</h3><p>{slide.copy}</p></div>
            </figcaption>
          </figure>
        ))}
      </div>
      <button className="carousel-arrow previous" type="button" onClick={() => move(-1)} aria-label="上一张店内环境图">←</button>
      <button className="carousel-arrow next" type="button" onClick={() => move(1)} aria-label="下一张店内环境图">→</button>
      <div className="carousel-dots" aria-label="选择店内区域">
        {slides.map((slide, index) => (
          <button
            type="button"
            key={slide.title}
            className={active === index ? "active" : ""}
            onClick={() => setActive(index)}
            aria-label={`查看${slide.title}`}
            aria-current={active === index ? "true" : undefined}
          />
        ))}
      </div>
    </div>
  );
}
