import type { Context } from "@netlify/functions";
import { getData } from "../../data";

export default async (req: Request, context: Context) => {
  const data = await getData();
  return new Response(JSON.stringify(data));
};
