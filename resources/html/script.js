function showPage(id) {
  document.querySelectorAll(".page").forEach((p) => {
    p.classList.add("hidden");
    p.classList.remove("show");
  });

  const page = document.getElementById(id);

  if (page) {
    page.classList.remove("hidden");
    void page.offsetWidth;
    page.classList.add("show");

    const lines = page.querySelectorAll(".fade-line");
    lines.forEach((el, i) => {
      el.classList.remove("animate-line");
      el.style.animationDelay = `${i * 0.08}s`;
      void el.offsetWidth;
      el.classList.add("animate-line");
    });
  }

  const buttons = document.querySelectorAll(".menu button");
  buttons.forEach((btn) => {
    btn.classList.toggle("active", btn.dataset.page === id);
  });
}

window.addEventListener("DOMContentLoaded", () => {
  const persistentElements = document.querySelectorAll(
    "h1.fade-line, nav.menu .fade-line"
  );

  persistentElements.forEach((el, i) => {
    el.style.animationDelay = `${i * 0.08}s`;
    el.classList.add("animate-line");
  });
});
