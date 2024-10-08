import type { Context } from "@netlify/functions";
import { getData } from "../../data";

export default async (req: Request, context: Context) => {
  const data = await getData();
  data["water"]["lastPressed"] = Math.floor((Date.now() - data["water"]["lastPressed"]) / 1000);
  data["food"]["lastPressed"] = Math.floor((Date.now() - data["food"]["lastPressed"]) / 1000);
  data["poop"]["lastPressed"] = Math.floor((Date.now() - data["poop"]["lastPressed"]) / 1000);
  return new Response(JSON.stringify(data));
};
