import type { Context } from "@netlify/functions";
import { ID_TO_KEY, getData, updateData } from "../../data";

export default async (req: Request, context: Context) => {
  // get id query param from the request url using UrlSearchParams
  const url = new URL(req.url);
  const id = Number(url.searchParams.get("id"));

  const data = await getData();
  data[ID_TO_KEY[id]].lastPressed = Date.now();
  await updateData(data);

  return new Response(data[ID_TO_KEY[id]].lastPressed);
};
