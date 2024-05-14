const { DateTime } = luxon;

const panels = {
  poop: document.querySelector(".panel[data-id='poop']"),
  water: document.querySelector(".panel[data-id='water']"),
  food: document.querySelector(".panel[data-id='food']"),
};

function updatePanel(id, data) {
  panels[id].querySelector(".time").innerText = DateTime.fromMillis(
    data.lastPressed
  ).toRelative();
  if (Math.random() > 0.5) {
    panels[id].setAttribute("data-mode", "on");
  } else {
    panels[id].setAttribute("data-mode", "off");
  }
}

async function fetchData() {
  const res = await axios.get("/.netlify/functions/data");
  updatePanel("poop", res.data.poop);
  updatePanel("water", res.data.water);
  updatePanel("food", res.data.food);
}

function boot() {
  setInterval(fetchData, 5000);
  fetchData().catch((err) => console.error(err));
}
