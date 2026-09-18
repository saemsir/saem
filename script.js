document.addEventListener('DOMContentLoaded', function () {
  const lectures = [
    ['JEE ADVANCED', 'JEE Advanced Physics Paper Solution', 'Detailed solution and reasoning for advanced physics problems.', 'https://www.youtube.com/live/3TT1crLy9PQ?si=gyVhZIM5InkcNF4o'],
    ['JEE MAINS', 'JEE Mains Physics Paper Solution', 'Work through the paper and understand each answer.', 'https://www.youtube.com/live/3IbsP-95MRE?si=BYujEr7TzCAeNRiB'],
    ['NEET', 'NEET Physics Paper Solution', 'A clear, step-by-step paper discussion.', 'https://www.youtube.com/live/Ya75UNbuVaI?si=JN63seRXFpHVkw8d'],
    ['MECHANICS', "Newton's Law of Motion", 'See how force, motion and mechanics fit together.', 'https://www.youtube.com/live/ARLQwOAdqd0?si=HOgtkaaQIqB0UaC2'],
    ['ROTATIONAL MOTION', 'Angular Momentum', 'Explore rotational motion through intuition and equations.', 'https://youtu.be/iGLd6pqGu4w?si=UUK1ZToThHkY8fD_'],
    ['OPTICS', 'Refraction of Light', 'Understand why light bends between materials.', 'https://youtu.be/dD5CMx5GeSM?si=uSyKlizkeR7II3kT'],
    ['CBSE CLASS 12', 'CBSE 12th Physics PYQ', 'Practice previous-year questions concept-first.', 'https://www.youtube.com/live/FvA6a8dzL0o?si=KaM5WR5qZ9bzRz5C'],
    ['BIHAR BOARD', 'Bihar Board 12th PYQ', 'Prepare with guided previous-year solutions.', 'https://www.youtube.com/live/3RmLjiBsL1Q?si=8xGlEimTatNyZGV0'],
    ['NCERT', 'NCERT Clear', 'Make NCERT concepts clear and connected.', 'https://www.youtube.com/live/fpqUMPcvvmc?si=3dFFE43w-sMJs3x9']
  ];

  const playlists = [
    ['JEE ADVANCE', 'JEE Advance Paper Solution', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGmJCQv5QKkfGWBBCLtzgQr1&si=I7vTInci3mQOs9N2'],
    ['NEET', 'NEET Exam Paper Solution', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGm7bdSxNp61GfEgAgpcb7c3&si=EcrB9UqsoHXF4uZG'],
    ['CLASS 12', '12th DEEP Physics', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGmz5Nu8Eo3tbW7C6TWxg47M&si=w7Tp_LEO3NaNfjpT'],
    ['CLASS 11', '11th Morning Physics', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGnJopwUTNwm4zB88Stdj8sc&si=HwBloKVuYDkOz8OG'],
    ['CLASS 11', '11th Deep Physics', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGkgKaiK8vYlJZRkklOYwrZ8&si=hmjmuwMQQu10lkGX'],
    ['DAILY LEARNING', 'Evening Physics', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGm5vX6_yZjSzkwPIv5BOr7d&si=ZVGB_tkm3_Lu_-vY'],
    ['REVISION', 'Quick Revision: System of Particles', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGldL1j2dvWCqR-9J_d1rQTv&si=5iBtkozfmJv2Pw53'],
    ['STRATEGY', "Topper's Strategy", 'https://youtube.com/playlist?list=PLjEkQ_dvlIGlbjHCLVfFG1LOmz6DhCpOB&si=YqUhwj7uVVRb-o9a'],
    ['JEE', 'Clear JEE Physics', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGnUR6SCSdM0oApUY7jldn1R&si=oW1Z8Fck_6wSgLFZ'],
    ['NEET', 'Clear NEET Physics', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGnoVXe4df8IAa1Ka-wvy9qr&si=jX-Sus6vzYBF-GnI'],
    ['CLASS 10', '10th Quick Revision', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGnrLTOGX75hlPRQE2mMDkUT&si=0ppHTD3-aq98uUrV'],
    ['CLASS 9', '9th Quick Revision', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGk-Y6nr-ldTnnCr8uRftGiC&si=9ygXAY1V3050SBG5'],
    ['CBSE 12', 'CBSE 12th Board exam', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGlHZ0f8U3R3_i1t9YFURQex&si=J-_FE2Sh3OdO0_aR'],
    ['BIHAR BOARD', 'Bihar Board Exam Guess', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGk0GPyRclNXoJYx7kQ5OQLV&si=aBSlHE8EMinmehJV']
  ];

  function card(label, title, description, url, playlist) {
    const link = document.createElement('a');
    link.className = 'card'; link.href = url; link.target = '_blank'; link.rel = 'noopener';
    link.innerHTML = `<small>${label}</small><h3>${title}</h3><p>${description}</p><b>${playlist ? 'Open playlist' : 'Watch on YouTube'} ↗</b>`;
    return link;
  }

  const lectureCards = document.getElementById('lecture-cards');
  const playlistCards = document.getElementById('playlist-cards');
  if (lectureCards) { lectureCards.replaceChildren(...lectures.map((x, i) => card(`${x[0]} · ${String(i + 1).padStart(2, '0')}`, x[1], x[2], x[3], false))); }
  if (playlistCards) { playlistCards.replaceChildren(...playlists.map((x, i) => card(`${x[0]} · ${String(i + 1).padStart(2, '0')}`, x[1], 'A focused Saem Physics learning path.', x[2], true))); }

  // Add the second featured lesson beside Trigonometry Basics.
  const feature = document.querySelector('.feature');
  if (feature && !document.querySelector('.feature-extra')) {
    const extra = document.createElement('div'); extra.className = 'feature-extra';
    extra.appendChild(card('MATHEMATICS · FEATURED', 'Maths for Physics', 'A quadratic-focused lesson to strengthen the mathematics behind physics.', 'https://youtu.be/sy70l6lQ7p8?si=jRQS-N0R-76PNis7', false));
    feature.insertAdjacentElement('afterend', extra);
  }

  // Connect the topic library to the requested lessons and add three more topic paths.
  const topics = document.querySelectorAll('.topic');
  const topicData = [
    ['Mechanics', 'NCERT Questions: Laws of Motion', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGmj-GBjRI7WC_vzYn4-7Si5&si=hqmCPs2kr-OM5-mF'],
    ['Mathematical tools', 'NCERT Book: Units, Dimensions & Errors', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGk-_9QojNMbg6vhRrk1C0Yg&si=YWpAnwJcX90NDFev'],
    ['Problem-solving', 'Work, Energy and Power', 'https://www.youtube.com/live/94KWCWIpTWM?si=GXJzfDrZw8iYHCC8']
  ];
  topics.forEach((topic, i) => { if (topicData[i]) { topic.href = topicData[i][2]; const heading = topic.querySelector('h3'); if (heading) heading.textContent = topicData[i][0]; const text = topic.querySelector('p'); if (text) text.textContent = topicData[i][1]; } });
  const topicGrid = document.querySelector('.topic-grid');
  [['Quick Revision', 'Electrostatics', 'https://youtu.be/OzuS2LwJwzI?si=531yUfZV0wWUEuSS', 'blue'], ['Quick Revision', 'Gravitation', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGlDLayMTKe7kAf28RswrXAc&si=aWb3DhyKGrZKYCxZ', 'gold'], ['Modern Physics', 'Modern Physics playlist', 'https://youtube.com/playlist?list=PLjEkQ_dvlIGmYGbJoXdyKCa3eN6w0OrL0&si=FMKiyUA__LxuaSLx', 'pink-card']].forEach((item, i) => { if (topicGrid && !topicGrid.querySelector(`[data-extra-topic="${i}"]`)) { const a = document.createElement('a'); a.className = `topic ${item[3]}`; a.dataset.extraTopic = i; a.href = item[2]; a.target = '_blank'; a.rel = 'noopener'; a.innerHTML = `<span>0${i + 4}</span><h3>${item[0]}</h3><p>${item[1]}</p><b>Explore ↗</b>`; topicGrid.appendChild(a); } });

  // Keep resource links visible in a compact top navigation group.
  const nav = document.getElementById('nav');
  if (nav && !document.querySelector('.top-resources')) {
    const resources = document.createElement('span'); resources.className = 'top-resources'; resources.innerHTML = '<a href="https://t.me/SaemPhysics" target="_blank" rel="noopener">Notes</a><a href="https://forms.gle/xGCHZt8vsd7rDf9k9" target="_blank" rel="noopener">Admission</a><a href="https://meet170.webex.com/meet/saem" target="_blank" rel="noopener">Live</a>';
    nav.insertBefore(resources, nav.querySelector('.button'));
  }

  const style = document.createElement('style'); style.textContent = '.feature-extra{margin-top:1rem;max-width:520px}.feature-extra .card{min-height:180px}.top-resources{display:flex;gap:.65rem;align-items:center;border-left:1px solid var(--line);padding-left:1rem}.top-resources a{font-size:.72rem!important;color:var(--yellow)!important}.top-resources a:hover{color:var(--blue)!important}@media(max-width:720px){.top-resources{border-left:0;padding-left:0;gap:1rem}.feature-extra{max-width:none}}'; document.head.appendChild(style);

  const year = document.getElementById('year'); if (year) year.textContent = new Date().getFullYear();
  const menu = document.getElementById('menu');
  if (menu && nav) { menu.addEventListener('click', () => { const open = nav.classList.toggle('open'); menu.setAttribute('aria-expanded', String(open)); }); nav.querySelectorAll('a').forEach(a => a.addEventListener('click', () => { nav.classList.remove('open'); menu.setAttribute('aria-expanded', 'false'); })); }

  const revealItems = document.querySelectorAll('.reveal');
  if ('IntersectionObserver' in window) { const observer = new IntersectionObserver(entries => entries.forEach(entry => { if (entry.isIntersecting) { entry.target.classList.add('visible'); observer.unobserve(entry.target); } }), { threshold: .1 }); revealItems.forEach(item => observer.observe(item)); } else revealItems.forEach(item => item.classList.add('visible'));

  const canvas = document.getElementById('space');
  if (canvas && !window.matchMedia('(prefers-reduced-motion: reduce)').matches) { const ctx = canvas.getContext('2d'); let particles = []; function resize() { canvas.width = innerWidth; canvas.height = innerHeight; particles = Array.from({ length: Math.min(70, Math.floor(innerWidth / 18)) }, () => ({ x: Math.random() * canvas.width, y: Math.random() * canvas.height, vx: (Math.random() - .5) * .35, vy: (Math.random() - .5) * .35, r: Math.random() * 1.5 + .4 })); } function draw() { ctx.clearRect(0, 0, canvas.width, canvas.height); particles.forEach((p, i) => { p.x = (p.x + p.vx + canvas.width) % canvas.width; p.y = (p.y + p.vy + canvas.height) % canvas.height; ctx.fillStyle = 'rgba(104,217,243,.4)'; ctx.beginPath(); ctx.arc(p.x, p.y, p.r, 0, Math.PI * 2); ctx.fill(); for (let j = i + 1; j < particles.length; j++) { const q = particles[j], d = Math.hypot(p.x - q.x, p.y - q.y); if (d < 115) { ctx.strokeStyle = `rgba(104,217,243,${.1 * (1 - d / 115)})`; ctx.beginPath(); ctx.moveTo(p.x, p.y); ctx.lineTo(q.x, q.y); ctx.stroke(); } } }); requestAnimationFrame(draw); } addEventListener('resize', resize); resize(); draw(); }
});
