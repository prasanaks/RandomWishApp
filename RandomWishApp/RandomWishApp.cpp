#include <crow.h>
#include <fstream>
#include <unordered_map>
#include <json/json.h>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>

// File paths for wish data and IP mapping
const std::string WISHES_FILE = "wishes.json";
const std::string ASSIGNMENTS_FILE = "assignments.json";

// Structure to hold the Secret Santa data
struct SantaData {
    std::string name;
    std::string trigram;
    std::string wish;
};

// Function to load wishes from the JSON file
std::vector<SantaData> loadWishes() {
    std::ifstream file(WISHES_FILE);
    std::vector<SantaData> wishes;

    if (file) {
        Json::Value root;
        file >> root;
        for (const auto& entry : root["wishes"]) {
            SantaData data = {
                entry["name"].asString(),
                entry["trigram"].asString(),
                entry["wish"].asString()
            };
            wishes.push_back(data);
        }
    }
    else {
        std::cerr << "Error: Could not open " << WISHES_FILE << std::endl;
    }
    return wishes;
}

// Function to load existing IP-to-wish mappings
std::unordered_map<std::string, SantaData> loadAssignments() {
    std::ifstream file(ASSIGNMENTS_FILE);
    std::unordered_map<std::string, SantaData> assignments;

    if (file) {
        Json::Value root;
        file >> root;
        for (const auto& ip : root.getMemberNames()) {
            SantaData data = {
                root[ip]["name"].asString(),
                root[ip]["trigram"].asString(),
                root[ip]["wish"].asString()
            };
            assignments[ip] = data;
        }
    }
    return assignments;
}

// Function to save IP-to-wish mappings
void saveAssignments(const std::unordered_map<std::string, SantaData>& assignments) {
    Json::Value root;
    for (const auto& pair : assignments) {
        Json::Value entry;
        entry["name"] = pair.second.name;
        entry["trigram"] = pair.second.trigram;
        entry["wish"] = pair.second.wish;
        root[pair.first] = entry;
    }

    std::ofstream file(ASSIGNMENTS_FILE);
    if (file) {
        file << root;
    }
    else {
        std::cerr << "Error: Could not open " << ASSIGNMENTS_FILE << std::endl;
    }
}

int main() {
    // Load wishes and assignments
    auto wishes = loadWishes();
    auto assignments = loadAssignments();

    if (wishes.empty()) {
        std::cerr << "Error: No wishes loaded. Check " << WISHES_FILE << std::endl;
        return 1;
    }

    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([&](const crow::request& req) {
        // Try to retrieve the IP address from the "X-Forwarded-For" header first
        std::string user_ip = req.get_header_value("X-Forwarded-For");

        if (user_ip.empty()) {
            // As the 'remote_addr' doesn't exist in this version of Crow, fall back to a different approach:
            user_ip = req.get_header_value("RemoteAddr");  // Use this as fallback if possible
        }

        if (user_ip.empty()) {
            user_ip = "Unknown";  // Fallback if no IP is retrieved
        }

        // Check if the user already has an assigned wish
        if (assignments.find(user_ip) == assignments.end()) {
            // Assign a random wish
            if (!wishes.empty()) {
                srand(time(0));
                int random_index = rand() % wishes.size();
                assignments[user_ip] = wishes[random_index];
                wishes.erase(wishes.begin() + random_index);
                saveAssignments(assignments);
            }
            else {
                return crow::response(500, "No more wishes available.");
            }
        }

        // Display the assigned wish
        SantaData assigned_data = assignments[user_ip];
        std::string html =
            "<html>"
            "<body style='text-align: center; font-family: Arial, sans-serif;'>"
            "<h1>Secret Santa Wish</h1>"
            "<p><strong>Name:</strong> " + assigned_data.name + "</p>"
            "<p><strong>Trigram:</strong> " + assigned_data.trigram + "</p>"
            "<p><strong>Wish:</strong></p>"
            "<h2 style='color: green;'>" + assigned_data.wish + "</h2>"
            "<button onclick='window.location.reload()'>Click Me Again</button>"
            "</body>"
            "</html>";

        return crow::response(html);
        });

    app.port(8080).multithreaded().run();

    return 0;
}
