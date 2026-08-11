import { InteriorCarousel } from "./InteriorCarousel";

const services = [
  { icon: "泡", title: "基础净护", copy: "温和清洁、吹干梳毛、耳道清洁与指甲修剪", price: "¥88 起", tone: "mint" },
  { icon: "剪", title: "造型焕新", copy: "根据毛量与脸型设计，修出舒服又耐看的造型", price: "¥168 起", tone: "orange" },
  { icon: "养", title: "皮毛养护", copy: "深层去浮毛、毛发护理与肉垫保湿，重回蓬松", price: "¥128 起", tone: "blue" },
];

const steps = [
  ["01", "到店问诊", "了解宠物习惯与皮肤状态"],
  ["02", "专属洗护", "一宠一消毒，全程温柔陪伴"],
  ["03", "清爽回家", "反馈护理情况与居家建议"],
];

export default function Home() {
  return (
    <main>
      <header className="nav-shell">
        <a className="brand" href="#top" aria-label="爪爪洗护首页">
          <span className="brand-mark" aria-hidden="true">●</span>
          <span>爪爪洗护 <small>PAW PAW</small></span>
        </a>
        <nav aria-label="主导航">
          <a href="#services">洗护服务</a>
          <a href="#interior">店内环境</a>
          <a href="#process">安心流程</a>
          <a href="#about">关于我们</a>
        </nav>
        <a className="button button-small" href="#booking">预约洗护 <span>↗</span></a>
      </header>

      <section className="hero" id="top">
        <div className="hero-copy">
          <div className="eyebrow"><span>✦</span> 专注宠物情绪的温柔洗护</div>
          <h1>洗得干净，<br />也要玩得<span>开心</span>。</h1>
          <p>不催促、不强迫。用耐心和专业，给每一只毛孩子一场轻松自在的洗护体验。</p>
          <div className="hero-actions">
            <a className="button" href="#booking">立即预约 <span>↗</span></a>
            <a className="text-link" href="#services">查看服务 <span>↓</span></a>
          </div>
          <div className="social-proof">
            <div className="avatars" aria-hidden="true"><i>🐶</i><i>🐱</i><i>🐕</i></div>
            <div><b>4.9 <span>★★★★★</span></b><small>来自 600+ 位毛孩子家长</small></div>
          </div>
        </div>

        <div className="hero-visual" aria-label="一只洗完澡、戴着橙色围巾的开心狗狗插画">
          <span className="bubble bubble-one">✦</span>
          <span className="bubble bubble-two">✦</span>
          <div className="stamp">温柔<br /><b>洗护</b></div>
          <div className="pet-card">
            <div className="pet-ears"><i /><i /></div>
            <div className="pet-face">
              <div className="eyes"><i /><i /></div>
              <div className="muzzle"><span>●</span><small>⌣</small></div>
            </div>
            <div className="bandana">PAW PAW</div>
          </div>
          <div className="mini-note"><span>♥</span><div><b>今日状态</b><small>蓬松 · 开心 · 满分</small></div></div>
        </div>
      </section>

      <section className="trust-strip" aria-label="服务承诺">
        <div><span>01</span><b>一宠一消毒</b><small>专属工具，安心卫生</small></div>
        <div><span>02</span><b>全程可视</b><small>透明护理，随时放心</small></div>
        <div><span>03</span><b>持证美容师</b><small>懂护理，更懂毛孩子</small></div>
      </section>

      <section className="section services" id="services">
        <div className="section-heading">
          <div><div className="eyebrow">我们的服务</div><h2>从鼻尖到尾巴，<br />都被认真照顾</h2></div>
          <p>根据宠物的品种、毛发和皮肤状态定制护理方案，不做流水线式服务。</p>
        </div>
        <div className="service-grid">
          {services.map((service) => (
            <article className={`service-card ${service.tone}`} key={service.title}>
              <span className="service-icon">{service.icon}</span>
              <h3>{service.title}</h3>
              <p>{service.copy}</p>
              <div><b>{service.price}</b><a href="#booking" aria-label={`预约${service.title}`}>了解详情 <span>→</span></a></div>
            </article>
          ))}
        </div>
      </section>

      <section className="section interior" id="interior">
        <div className="section-heading">
          <div><div className="eyebrow">店内环境</div><h2>每一处空间，<br />都为安心而设计</h2></div>
          <p>明亮、洁净，也保留足够的松弛感。三个独立区域，让接待、洗护和休息互不打扰。</p>
        </div>
        <InteriorCarousel />
      </section>

      <section className="section process" id="process">
        <div className="process-card">
          <div className="process-copy">
            <div className="eyebrow light">安心可见</div>
            <h2>三步完成一次<br />轻松的洗护</h2>
            <p>流程透明、用品透明、状态透明。每一次接触，都尊重毛孩子的感受。</p>
            <a className="button button-light" href="#booking">为毛孩子预约 <span>↗</span></a>
          </div>
          <div className="steps">
            {steps.map(([number, title, copy]) => (
              <div className="step" key={number}><span>{number}</span><div><h3>{title}</h3><p>{copy}</p></div></div>
            ))}
          </div>
        </div>
      </section>

      <section className="section about" id="about">
        <div className="quote-mark">“</div>
        <blockquote>真正好的洗护，不只是变漂亮，<br />更是让它<span>下次还愿意来</span>。</blockquote>
        <div className="reviewer"><span>徐</span><div><b>徐小雨 · 店长美容师</b><small>8 年宠物洗护经验 / A 级宠物美容师</small></div></div>
      </section>

      <section className="booking" id="booking">
        <div>
          <div className="eyebrow">现在预约</div>
          <h2>让毛孩子，<br />清清爽爽去撒欢。</h2>
          <p>首次到店赠送皮毛检测与护理建议。</p>
        </div>
        <div className="booking-info">
          <div><small>营业时间</small><b>每天 09:30 — 19:30</b></div>
          <div><small>门店地址</small><b>成都太古里</b></div>
          <a className="button button-dark" href="tel:4008881024">拨打 400-888-1024 <span>↗</span></a>
        </div>
      </section>

      <section className="location" id="location">
        <div className="location-image">
          <img src="/location-taikoo-li-real.webp" alt="绿树与灰瓦建筑相映的成都太古里步行街" />
          <small>街区环境示意 · 实际路线以地图导航为准</small>
        </div>
        <div className="location-copy">
          <div className="eyebrow">到店指南</div>
          <h2>我们在<br />成都太古里</h2>
          <p>藏在城市中心的一处温暖小店。逛街、喝咖啡的时候，也顺路带毛孩子来洗个舒服的澡。</p>
          <div className="location-meta">
            <span><small>ADDRESS</small><b>四川省成都市锦江区 · 成都太古里</b></span>
            <span><small>OPENING HOURS</small><b>每天 09:30 — 19:30</b></span>
          </div>
          <a className="button button-dark" href="https://uri.amap.com/search?keyword=%E6%88%90%E9%83%BD%E5%A4%AA%E5%8F%A4%E9%87%8C" target="_blank" rel="noreferrer">打开地图导航 <span>↗</span></a>
        </div>
      </section>

      <footer>
        <a className="brand" href="#top"><span className="brand-mark">●</span><span>爪爪洗护 <small>PAW PAW</small></span></a>
        <p>把每一只毛孩子，当作自己的家人。</p>
        <span>© 2026 PAW PAW PET CARE</span>
      </footer>
    </main>
  );
}
