// See https://aka.ms/new-console-template for more information
// Console.WriteLine("Hello, World!");
// Laser Catnip project - Bluetooth Mobile App simulator for testing ESP32 Bluetooth capabilities

using System.Drawing;
using System.Net.Sockets;
using System.Text;
using InTheHand.Net.Bluetooth;
using InTheHand.Net.Sockets;
using Microsoft.VisualBasic;
using Newtonsoft.Json;

internal class Program
{
    private static Thread? readThread = null; // Thread variable
    private static NetworkStream? stream;
    private static void Main(string[] args)
    {

        while (true)
        {
            try
            {
                BluetoothClient client = new();
                Console.WriteLine("Scanning for Bluetooth Devices");

                BluetoothDeviceInfo[] devices;// = client.DiscoverDevices().ToArray();
                try
                {
                    //BluetoothClient client = new BluetoothClient();
                    devices = client.DiscoverDevices().ToArray();
                    // If no exception is thrown, proceed with the rest of your code...
                }
                catch (PlatformNotSupportedException)
                {
                    Console.WriteLine("Bluetooth is not supported on this device");
                    Console.ReadKey();
                    return;
                }
                catch (SocketException ex)
                {
                    // This could indicate that Bluetooth is turned off or not functioning properly
                    Console.WriteLine("An error occurred: " + ex.Message);
                    Console.WriteLine("Please ensure Bluetooth is turned on.");
                    Console.ReadKey();
                    return;
                }
                catch (Exception ex)
                {
                    // Handle other general exceptions
                    Console.WriteLine("An unexpected error occurred: " + ex.Message);
                    Console.ReadKey();
                    return;
                }


                if (devices.Length == 0)
                {
                    Console.WriteLine("No BT Devices discovered, check Bluetooth On.");
                    Task.Delay(1000);
                    continue;
                }
                else
                    Console.WriteLine(
                        string.Join(", ", new string[] { "Devices: " }.Concat(devices.Select(e => e.DeviceName))));

                BluetoothDeviceInfo? esp32Device = devices.FirstOrDefault(d => d.DeviceName == "ESP32Test");
                if (esp32Device != null)
                {
                    string jsonString = JsonConvert.SerializeObject(esp32Device, Formatting.Indented);
                    Console.WriteLine("Discovered ESP32 controller: \n" + jsonString);

                    //esp32Device.Dump("esp32Device");
                    client.Connect(esp32Device.DeviceAddress, BluetoothService.SerialPort);

                    stream = client.GetStream();

                    // Start the read thread only if it's not already running
                    if (readThread == null || !readThread.IsAlive)
                    {
                        readThread = new Thread(() =>
                        {
                            while (true)
                            {
                                if (stream.CanRead)
                                {
                                    byte[] myReadBuffer = new byte[1024];
                                    StringBuilder myCompleteMessage = new();
                                    int numberOfBytesRead = 0;

                                    do
                                    {
                                        try
                                        {
                                            numberOfBytesRead = stream.Read(myReadBuffer, 0, myReadBuffer.Length);
                                            myCompleteMessage.AppendFormat("{0}", Encoding.ASCII.GetString(myReadBuffer, 0, numberOfBytesRead));
                                        }
                                        catch { throw; };
                                    }
                                    while (stream.DataAvailable);

                                    Console.WriteLine($"ESP32: '{myCompleteMessage}'");
                                }
                                Thread.Sleep(100); // Avoids overwhelming the ESP32 and the CPU
                            }
                        })
                        { IsBackground = true };
                        readThread.Start();
                    }

                    // Create an anonymous object that matches the required structure.
                    var commandObject = new
                    {
                        command = "set_date_time",
                        date_time = DateTime.Now.ToString("yyyy-MM-ddTHH:mm:ss")
                    };
                    // Convert the anonymous object to a JSON string.
                    string jsonStringWithTime = JsonConvert.SerializeObject(commandObject) + "\n";

                    if (stream.CanWrite)
                    {
                        Console.Write($"Sending to the controller: {jsonStringWithTime}");
                        byte[] message = Encoding.ASCII.GetBytes(jsonStringWithTime);
                        stream.Write(message, 0, message.Length);
                    }

                    // Define a maximum chunk size
                    const int maxChunkSize = 100; // Adjust this value based on testing

                    while (true) // Continuous loop
                    {
                        // Sending data to ESP32
                        if (stream.CanWrite)
                        {
                            Console.WriteLine("Enter command to send (type 'exit' to quit, or 'file' to send a file): ");
                            string? input = Console.ReadLine();
                            if (input == "exit") break;
                            if (input == "file")
                            {
                                SendFile();
                                continue;
                            }
                            if (input == null) continue;

                            // Break the input into chunks
                            for (int i = 0; i < input.Length; i += maxChunkSize)
                            {
                                string chunk = input.Substring(i, Math.Min(maxChunkSize, input.Length - i));
                                byte[] message = Encoding.ASCII.GetBytes(chunk);
                                stream.Write(message, 0, message.Length);
                                Thread.Sleep(50); // Small delay between chunks
                            }

                            // Send a newline character at the end
                            stream.WriteByte((byte)'\n');
                        }

                        // Optional: Add a delay to prevent overwhelming the ESP32
                        Thread.Sleep(10);
                    }

                    client.Close();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Unrecognized exception {ex.Message}");

                // If an exception occurs, safely abort or finish the read thread
                if (readThread != null && readThread.IsAlive)
                {
                    readThread.Abort();
                }

                // Reset the thread variable
                readThread = null;
            }
        }
    }
    static private byte[] fileImage = new byte[] {
    2, 0, 0, 0, 16, 0, 0, 0, 152, 31, 189, 94, 140, 1, 0, 0,
    5, 0, 0, 0, 32, 0, 0, 0, 5, 0, 0, 0, 57, 0, 0, 0, 82, 0, 0, 0,
    67, 100, 128, 12, 232, 3, 92, 0, 0, 0, 67, 200, 64, 6, 232, 3,
    150, 32, 8, 232, 3, 130, 64, 11, 232, 3, 102, 0, 0, 0, 67, 230,
    128, 17, 232, 3, 230, 64, 11, 232, 3, 112, 0, 0, 0, 67, 100, 0,
    100, 0, 232, 3, 0, 0, 4, 0, 200, 0, 200, 0, 232, 3, 1, 0, 2, 0,
    130, 0, 24, 1, 232, 3, 0, 0, 1, 0, 180, 0, 150, 0, 232, 3, 1, 0, 0, 0};

    private static void SendFile()
    {
        if (stream == null) return;

        var commandObject = new
        {
            command = "download_file_image",
            storage_name = "path_manager_storage.bin",
            storage_size = fileImage.Length,
            control_sum = CalculateControlSum()
        };
        // Convert the anonymous object to a JSON string.
        var j_command_string = JsonConvert.SerializeObject(commandObject) + "\n";

        if (stream.CanWrite)
        {
            Console.Write($"Sending to the controller: {j_command_string}");
            byte[] message = Encoding.ASCII.GetBytes(j_command_string);
            stream.Write(message, 0, message.Length);
        }

        // stuff the buffer with the json command \n
        // add a binary trailer to the end of the command, which will be interpreted/stored as a file on the ESP32 side.
        stream.Write(fileImage, 0, fileImage.Length);
    }

    static byte CalculateControlSum()
    {
        int sum = 0;
        foreach (byte b in fileImage)
        {
            sum += b;
            sum &= 0xFF;
        }
        return (byte)sum;
    }
}

