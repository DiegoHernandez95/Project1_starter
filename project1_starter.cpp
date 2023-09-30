#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

// Represents a time slot with start and end times.
struct TimeSlot {
    int start, end;
};

// turn the time values into integers
int parseTime(const std::string& timeStr) {
    try {
        int hour = std::stoi(timeStr.substr(0, 2));
        int minute = std::stoi(timeStr.substr(3, 2));
        return hour * 60 + minute;
    }
    catch (const std::exception& e) {
        throw std::invalid_argument("Error parsing time: " + timeStr);
    }
} // end parseTime

std::string formatTime(int minutes_total) {
    int hours = minutes_total / 60;
    int minutes = minutes_total % 60;
    return std::to_string(hours) + ":" + (minutes < 10 ? "0" : "") + std::to_string(minutes);
} // end formatTime

std::vector<TimeSlot> parseBusySlots(const std::string& line) {
    std::vector<TimeSlot> busy;
    size_t pos = 0, end = 0;
    while ((pos = line.find("['", pos)) != std::string::npos) {
        end = line.find("']", pos);
        if (end == std::string::npos) break;
        std::string time = line.substr(pos + 2, end - pos - 2);
        size_t sep = time.find("':'");
        busy.push_back({ parseTime(time.substr(0, sep)), parseTime(time.substr(sep + 3)) });
        pos = end + 2;
    }
    return busy;
} // end parseBusySlots

// get the working hours
TimeSlot parseWorkHours(const std::string& line) {
    size_t pos = line.find("['");
    size_t end = line.find("']");
    std::string time = line.substr(pos + 2, end - pos - 2);
    size_t sep = time.find("','");
    return { parseTime(time.substr(0, sep)), parseTime(time.substr(sep + 3)) };
} // end parseWorkHours

// turn the duration of the meeting into an integer
int parseDuration(const std::string& line) {
    return std::stoi(line);
} // end parseDuration

// Merge time slots and find available times for meetings
std::vector<TimeSlot> getAvailableSlots(const std::vector<TimeSlot>& busySlots, const TimeSlot& workPeriod) {
    std::vector<TimeSlot> availSlots;
    int end = 0;

    // Iterate through busy slots and working hours
    for (const auto& slot : busySlots) {
        if (slot.start > end) {
            availSlots.push_back({ end, slot.start });
        }
        end = std::max(end, slot.end);
    }

    // Check if there's available time after the last busy slot
    if (end < workPeriod.end) {
        availSlots.push_back({ end, workPeriod.end });
    }

    return availSlots;
}

// Find intersection of available slots of two persons.
std::vector<TimeSlot> intersectSlots(const std::vector<TimeSlot>& a, const std::vector<TimeSlot>& b) {
    std::vector<TimeSlot> intersect;
    int i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        int start = std::max(a[i].start, b[j].start);
        int end = std::min(a[i].end, b[j].end);
        if (start < end) {
            intersect.push_back({ start, end });
        }
        if (a[i].end < b[j].end) {
            ++i;
        }
        else {
            ++j;
        }
    }
    return intersect;
} // end intersectSlots

// Find meeting slots 
std::vector<TimeSlot> findMeetingSlots(const std::vector<TimeSlot>& avail, int duration) {
    std::vector<TimeSlot> meetings;
    for (const auto& slot : avail) {
        if (slot.end - slot.start >= duration) {
            meetings.push_back(slot);
        }
    }
    return meetings;
} // end findMeetingSlots

int main() {
    std::ifstream inputFile("input.txt");
    std::ofstream outputFile("output.txt");

    std::string line;
    while (true) {  // Continue until break.
        try {
            // Parse Person A's busy time slots
            if (!std::getline(inputFile, line)) break;
            std::vector<TimeSlot> busyA = parseBusySlots(line);

            // Parse Person A's working hours
            if (!std::getline(inputFile, line)) break;
            TimeSlot workHoursA = parseWorkHours(line);

            // Parse Person B's busy time slots
            if (!std::getline(inputFile, line)) break;
            std::vector<TimeSlot> busyB = parseBusySlots(line);

            // Parse Person B's working hours
            if (!std::getline(inputFile, line)) break;
            TimeSlot workHoursB = parseWorkHours(line);

            // Parse meeting duration
            if (!std::getline(inputFile, line)) break;
            int duration = parseDuration(line);

            // Find available slots
            auto availableA = getAvailableSlots(busyA, workHoursA);
            auto availableB = getAvailableSlots(busyB, workHoursB);

            // Find intersection
            auto intersection = intersectSlots(availableA, availableB);

            // Find possible meeting slots
            auto meetingSlots = findMeetingSlots(intersection, duration);

            outputFile << "Available meeting slots:\n";
            if (meetingSlots.empty()) {
                outputFile << "There are no available meeting times.\n";
            }
            else {
                for (const auto& slot : meetingSlots) {
                    outputFile << "[" << formatTime(slot.start) << " - " << formatTime(slot.end) << "]" << "\n";
                }
            }
            outputFile << "\n";
        }
        catch (const std::exception& e) {
            outputFile << "Error: " << e.what() << "\n\n";
        }

        // deal with empty lines
        std::getline(inputFile, line);
    }

    inputFile.close();
    outputFile.close();
    return 0;
} // end main

