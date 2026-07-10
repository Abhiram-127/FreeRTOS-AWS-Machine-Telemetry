import { useEffect, useState } from "react";
import { getLatestData } from "../services/api";

import {
  Container,
  Box,
  Grid,
  Card,
  CardContent,
  Typography,
  Chip,
  Divider
} from "@mui/material";

import MemoryIcon from "@mui/icons-material/Memory";
import ThermostatIcon from "@mui/icons-material/Thermostat";
import WaterDropIcon from "@mui/icons-material/WaterDrop";
import GraphicEqIcon from "@mui/icons-material/GraphicEq";
import CheckCircleIcon from "@mui/icons-material/CheckCircle";

import {
  ResponsiveContainer,
  AreaChart,
  Area,
  CartesianGrid,
  XAxis,
  YAxis,
  Tooltip
} from "recharts";

function Dashboard() {

  const [sensor, setSensor] = useState(null);
  const [chartData, setChartData] = useState([]);
  const [time, setTime] = useState("");

  useEffect(() => {

    const updateClock = () =>
      setTime(new Date().toLocaleTimeString());

    updateClock();

    const clock = setInterval(updateClock, 1000);

    return () => clearInterval(clock);

  }, []);

  useEffect(() => {

    const loadData = async () => {

      try {

        const data = await getLatestData();

        setSensor(data);

        setChartData(prev => {

          const updated = [
            ...prev,
            {
              time: new Date().toLocaleTimeString(),
              vibration: data.vibration
            }
          ];

          return updated.slice(-25);

        });

      }

      catch (err) {

        console.log(err);

      }

    };

    loadData();

    const timer = setInterval(loadData, 2000);

    return () => clearInterval(timer);

  }, []);

  if (!sensor)
    return <Typography>Loading...</Typography>;

  return (

    <Box sx={{ background: "#0f172a", minHeight: "100vh", pb: 5 }}>

      {/* NAVBAR */}

      <Box
        sx={{
          background: "#1976d2",
          color: "white",
          px: 4,
          py: 2,
          display: "flex",
          justifyContent: "space-between",
          alignItems: "center"
        }}
      >

        <Box display="flex" alignItems="center">

          <MemoryIcon sx={{ mr: 2, fontSize: 32 }} />

          <Typography
            variant="h5"
            fontWeight="bold">

            Industrial Machine Health Monitor

          </Typography>

        </Box>

        <Box display="flex" gap={2} alignItems="center">

          <Chip
            color="success"
            label="ONLINE"
          />

          <Chip
            label="ESP32-S3"
            color="primary"
            variant="outlined"
          />

          <Typography>

            {time}

          </Typography>

        </Box>

      </Box>

      <Container maxWidth="xl" sx={{ mt: 4 }}>

        <Grid container spacing={3}>

          {/* Temperature */}

          <Grid size={{ xs: 12, sm: 6, md: 3 }}>

            <Card sx={{ borderRadius: 3 }}>

              <CardContent>

                <ThermostatIcon
                  color="error"
                  sx={{ fontSize: 40 }}
                />

                <Typography
                  color="text.secondary">

                  Temperature

                </Typography>

                <Typography
                  variant="h4"
                  fontWeight="bold">

                  {sensor.temperature} °C

                </Typography>

              </CardContent>

            </Card>

          </Grid>

          {/* Humidity */}

          <Grid size={{ xs: 12, sm: 6, md: 3 }}>

            <Card sx={{ borderRadius: 3 }}>

              <CardContent>

                <WaterDropIcon
                  color="primary"
                  sx={{ fontSize: 40 }}
                />

                <Typography
                  color="text.secondary">

                  Humidity

                </Typography>

                <Typography
                  variant="h4"
                  fontWeight="bold">

                  {sensor.humidity} %

                </Typography>

              </CardContent>

            </Card>

          </Grid>

          {/* Vibration */}

          <Grid size={{ xs: 12, sm: 6, md: 3 }}>

            <Card sx={{ borderRadius: 3 }}>

              <CardContent>

                <GraphicEqIcon
                  color="warning"
                  sx={{ fontSize: 40 }}
                />

                <Typography
                  color="text.secondary">

                  Vibration

                </Typography>

                <Typography
                  variant="h4"
                  fontWeight="bold">

                  {sensor.vibration.toFixed(2)}

                </Typography>

              </CardContent>

            </Card>

          </Grid>

          {/* Status */}

          <Grid size={{ xs: 12, sm: 6, md: 3 }}>

            <Card
              sx={{
                borderRadius: 3,
                bgcolor: sensor.alert ? "#b71c1c" : "#1b5e20",
                color: "white"
              }}
            >

              <CardContent>

                <CheckCircleIcon
                  sx={{ fontSize: 40 }}
                />

                <Typography>

                  Machine Status

                </Typography>

                <Typography
                  variant="h4"
                  fontWeight="bold">

                  {sensor.alert ? "ALERT" : "NORMAL"}

                </Typography>

              </CardContent>

            </Card>

          </Grid>
                    {/* Live Graph */}

          <Grid size={12}>

            <Card
              sx={{
                borderRadius: 3,
                height: 430
              }}
            >

              <CardContent>

                <Typography
                  variant="h5"
                  fontWeight="bold"
                  gutterBottom>

                  Live Machine Vibration

                </Typography>

                <Divider sx={{ mb: 2 }} />

                <ResponsiveContainer width="100%" height={320}>

                  <AreaChart data={chartData}>

                    <defs>

                      <linearGradient
                        id="colorVib"
                        x1="0"
                        y1="0"
                        x2="0"
                        y2="1">

                        <stop
                          offset="5%"
                          stopColor="#00E5FF"
                          stopOpacity={0.4}
                        />

                        <stop
                          offset="95%"
                          stopColor="#00E5FF"
                          stopOpacity={0}
                        />

                      </linearGradient>

                    </defs>

                    <CartesianGrid
                      stroke="#334155"
                      strokeDasharray="4 4"
                    />

                    <XAxis
                      dataKey="time"
                      minTickGap={35}
                      tick={{
                        fill: "#94A3B8",
                        fontSize: 11
                      }}
                    />

                    <YAxis
                      tick={{
                        fill: "#94A3B8"
                      }}
                    />

                    <Tooltip
                      contentStyle={{
                        background: "#1E293B",
                        border: "none",
                        borderRadius: 10,
                        color: "#fff"
                      }}
                    />

                    <Area
                      type="monotone"
                      dataKey="vibration"
                      stroke="none"
                      fill="url(#colorVib)"
                    />

                    <Area
                      type="monotone"
                      dataKey="vibration"
                      stroke="#00E5FF"
                      strokeWidth={3}
                      fillOpacity={0}
                    />

                  </AreaChart>

                </ResponsiveContainer>

              </CardContent>

            </Card>

          </Grid>

          {/* Bottom Row */}

          <Grid size={{ xs: 12, md: 6 }}>

            <Card sx={{ borderRadius: 3 }}>

              <CardContent>

                <Typography
                  variant="h5"
                  fontWeight="bold"
                  gutterBottom>

                  Accelerometer

                </Typography>

                <Divider sx={{ mb: 2 }} />

                <Box
                  display="flex"
                  justifyContent="space-between"
                  py={1}>

                  <Typography>X Axis</Typography>

                  <Typography fontWeight="bold">

                    {sensor.accel_x}

                  </Typography>

                </Box>

                <Box
                  display="flex"
                  justifyContent="space-between"
                  py={1}>

                  <Typography>Y Axis</Typography>

                  <Typography fontWeight="bold">

                    {sensor.accel_y}

                  </Typography>

                </Box>

                <Box
                  display="flex"
                  justifyContent="space-between"
                  py={1}>

                  <Typography>Z Axis</Typography>

                  <Typography fontWeight="bold">

                    {sensor.accel_z}

                  </Typography>

                </Box>

              </CardContent>

            </Card>

          </Grid>

          <Grid size={{ xs: 12, md: 6 }}>

            <Card sx={{ borderRadius: 3 }}>

              <CardContent>

                <Typography
                  variant="h5"
                  fontWeight="bold"
                  gutterBottom>

                  Gyroscope

                </Typography>

                <Divider sx={{ mb: 2 }} />

                <Box
                  display="flex"
                  justifyContent="space-between"
                  py={1}>

                  <Typography>X Axis</Typography>

                  <Typography fontWeight="bold">

                    {sensor.gyro_x}

                  </Typography>

                </Box>

                <Box
                  display="flex"
                  justifyContent="space-between"
                  py={1}>

                  <Typography>Y Axis</Typography>

                  <Typography fontWeight="bold">

                    {sensor.gyro_y}

                  </Typography>

                </Box>

                <Box
                  display="flex"
                  justifyContent="space-between"
                  py={1}>

                  <Typography>Z Axis</Typography>

                  <Typography fontWeight="bold">

                    {sensor.gyro_z}

                  </Typography>

                </Box>

              </CardContent>

            </Card>

          </Grid>
                    {/* Alerts */}

          <Grid size={12}>

            <Card
              sx={{
                borderRadius: 3,
                borderLeft: sensor.alert
                  ? "6px solid #ef4444"
                  : "6px solid #22c55e"
              }}
            >

              <CardContent>

                <Typography
                  variant="h5"
                  fontWeight="bold"
                  gutterBottom>

                  Recent Alerts

                </Typography>

                <Divider sx={{ mb: 2 }} />

                <Typography
                  variant="h6"
                  color={
                    sensor.alert
                      ? "error.main"
                      : "success.main"
                  }
                >

                  {sensor.alert
                    ? "⚠ High Vibration Detected!"
                    : "✔ System Operating Normally"}

                </Typography>

              </CardContent>

            </Card>

          </Grid>

        </Grid>

      </Container>

    </Box>

  );

}

export default Dashboard;