const { DateTime } = luxon;

const panels = {
  poop: {
    element: document.querySelector(".panel[data-id='poop']"),
    delay: 5000,
  },
  water: {
    element: document.querySelector(".panel[data-id='water']"),
    delay: 5000,
  },
  food: {
    element: document.querySelector(".panel[data-id='food']"),
    delay: 5000,
  },
};

function updatePanel(id, data) {
  const element = panels[id].element;
  element.querySelector(".time").innerText = DateTime.fromMillis(
    data.lastPressed
  ).toRelative();
  if (Date.now() - data.lastPressed >= panels[id].delay) {
    element.setAttribute("data-mode", "on");
  } else {
    element.setAttribute("data-mode", "off");
  }
}

async function fetchData() {
  const res = await axios.get("/.netlify/functions/data");
  updatePanel("poop", res.data.poop);
  updatePanel("water", res.data.water);
  updatePanel("food", res.data.food);
}

function boot() {
  setInterval(fetchData, 10000);
  fetchData().catch((err) => console.error(err));
}

boot();
