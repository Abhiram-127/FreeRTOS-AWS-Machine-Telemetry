import axios from "axios";

const API = axios.create({
    baseURL: "https://oz103s2jna.execute-api.us-east-1.amazonaws.com/prod"
});

export const getLatestData = async () => {
    const response = await API.get("/latest");
    return response.data;
};