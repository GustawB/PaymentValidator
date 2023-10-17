#include <iostream>
#include <string>
#include <sstream>
#include <regex>
#include <vector>
#include <unordered_map>

using std::string;
using std::regex;
using std::regex_match;
using std::cout;
using std::cerr;
using std::endl;
using std::smatch;
using std::getline;
using std::cin;
using parkedCarsData = std::unordered_map<string, int32_t>;
using dataInput = std::vector<string>;

/*
* Namespace containing all variables related to parking time.
* Data is kept in MINUTES!!!
*/
namespace ParkingTimeConstants {
    const int32_t HOUR = 60;
    const int32_t PARKING_UPPER_LIMIT = 719;
    const int32_t PARKING_LOWER_LIMIT = 10;
    const int32_t PARKING_UNTIL_NEXT_DAY_ADJUSTMENT = 720;
}

/*
* Namespace containing all regex patterns required for input validation.
*/
namespace Regex {
    //Strings used to build regex patterns.
    const static string registrationPattern = "[A-Z][0-9A-Z]{2,10}";
    const static string timePattern =
        "0?[8-9]\\.[0-5][0-9]|1[0-9]\\.[0-5][0-9]|20\\.00";
    //Regex patterns.
    const static auto paymentRegistrationPattern =
        regex("\\s*(" + registrationPattern + ")\\s+("
            + timePattern + ")\\s+(" + timePattern + ")\\s*");
    const static auto paymentValidationPattern =
        regex("\\s*(" + registrationPattern + ")\\s+("
            + timePattern + ")\\s*");
}

/*
* Function used to translate a string containing information about time to
* int32_t representing the number of minutes in the given 'time'
* (e.g. 21.37 has 1297 minutes).
*/
int32_t ConvertTimeToMinutes(const string& time) {
    int32_t minutes;
    if (time[1] == '.') {
        minutes = (time[0] - '0') * ParkingTimeConstants::HOUR;
        minutes += (time[2] - '0') * 10 + (time[3] - '0');
    }
    else {
        minutes = ((time[0] - '0') * 10 + time[1] - '0') *
            ParkingTimeConstants::HOUR;
        minutes += (time[3] - '0') * 10 + (time[4] - '0');
    }
    return minutes;
}

/*
* Function responsible for processing one line of input; First, by checking
* whether it is valid or not, and if so, adding data to the appropriate
* data structures.
*/
bool ParseLineIntoData(const string& oneLineInput,
    dataInput& parsedOneLineInput) {
    smatch dataFields;//data container
    if (regex_match(oneLineInput, dataFields,
        Regex::paymentRegistrationPattern)) {
        //We received a payment as the input
        parsedOneLineInput.push_back(dataFields[1].str());
        parsedOneLineInput.push_back(dataFields[2].str());
        parsedOneLineInput.push_back(dataFields[3].str());
        return true;
    }
    else if (regex_match(oneLineInput, dataFields,
        Regex::paymentValidationPattern)) {
        //We received parking validation as the input
        parsedOneLineInput.push_back(dataFields[1].str());
        parsedOneLineInput.push_back(dataFields[2].str());
        return true;
    }
    //We received garbage as the input
    return false;
}

/*
* Function responsible for checking whether the parking time
* isn't too short or too long. If it is, this function
* return false, and true otherwise.
*/
bool IsParkingTimeValid(int32_t beg, int32_t end) {
    int32_t parkingLength = end - beg;
    if (parkingLength < 0) {
        parkingLength += ParkingTimeConstants::PARKING_UNTIL_NEXT_DAY_ADJUSTMENT;
    }
    if (parkingLength >= ParkingTimeConstants::PARKING_LOWER_LIMIT &&
        parkingLength <= ParkingTimeConstants::PARKING_UPPER_LIMIT) {
        return true;
    }
    else { return false; }
}

/*
* Function responsible for registering the payment. It's done by comparing
* the beg and end time of the parking. Because the upper limit for parking
* is 719 min, if the end time is smaller than the beg time, we add the
* parking info to the map containing info about cars that will be parked
* until tomorrow. Otherwise, we update the map containing the info
* about parkings that will end today.
*/
void ReceivingPayment(parkedCarsData& carsParkedUntilToday,
    parkedCarsData& carsParkedUntilTomorrow,
    const dataInput& oneLineInput, int32_t lineCounter) {
    int32_t beg = ConvertTimeToMinutes(oneLineInput[1]);
    int32_t end = ConvertTimeToMinutes(oneLineInput[2]);
    int32_t previousEnd = 0;
    if (beg <= end && !carsParkedUntilTomorrow.contains(oneLineInput[0])) {//Parking will end today.
        if (carsParkedUntilToday.contains(oneLineInput[0])) {
            previousEnd = carsParkedUntilToday.at(oneLineInput[0]);
        }
        carsParkedUntilToday[oneLineInput[0]] = std::max(end, previousEnd);
    }
    else if (beg > end) {//Parking will end tomorrow.
        if (carsParkedUntilTomorrow.contains(oneLineInput[0])) {
            previousEnd = carsParkedUntilTomorrow.at(oneLineInput[0]);
        }
        carsParkedUntilTomorrow[oneLineInput[0]] = std::max(end, previousEnd);
    }
    cout << "OK " << lineCounter << endl;
}

/*
* Function responsible for checking whether the given car is still authorized
* to park.
*/
void ValidatingPayment(const parkedCarsData& carsParkedUntilToday,
    const parkedCarsData& carsParkedUntilTomorrow,
    const dataInput& oneLineInput, int32_t lineCounter) {
    if (carsParkedUntilTomorrow.contains(oneLineInput[0])) {
        //Valid parking
        cout << "YES " << lineCounter << endl;
    }
    else if (carsParkedUntilToday.contains(oneLineInput[0])) {
        int32_t upperTimeLimit = carsParkedUntilToday.at(oneLineInput[0]);
        int32_t currentTime = ConvertTimeToMinutes(oneLineInput[1]);
        if (currentTime <= upperTimeLimit) {
            //There is still time left for parking.
            cout << "YES " << lineCounter << endl;
        }
        else {
            //Parking time ended.
            cout << "NO " << lineCounter << endl;
        }
    }
    else {
        //No data about the given registration.
        cout << "NO " << lineCounter << endl;
    }
}

/*
* Utility function that decides whether we are receiving payment or
* validating it and calls respective functions based on that.
*/
void ParkedCarProcessing(parkedCarsData& carsParkedUntilToday,
    parkedCarsData& carsParkedUntilTomorrow,
    const dataInput& oneLineInput, int32_t lineCounter) {
    if (oneLineInput.size() == 3) {//Receiving payment.
        ReceivingPayment(carsParkedUntilToday, carsParkedUntilTomorrow,
            oneLineInput, lineCounter);
    }
    else {//oneLineInput.size() == 2, validating payment.
        ValidatingPayment(carsParkedUntilToday, carsParkedUntilTomorrow,
            oneLineInput, lineCounter);
    }
}

/*
 * Function responsible for reading the data from input
 * and processing it.
 */
void MainLogicLoop() {
    parkedCarsData carsParkedUntilToday;
    parkedCarsData carsParkedUntilTomorrow;
    string line;
    //variable used to keep track of the day (today or tomorrow).
    int32_t currentTime = 0;
    int32_t nextCurrentTime;
    int32_t lineCounter = 0;
    while (getline(cin, line)) {//Reading input.
        lineCounter++;
        dataInput oneLineInput;
        if (!ParseLineIntoData(line, oneLineInput)) {
            cerr << "ERROR " << lineCounter << endl;
        }
        else if (oneLineInput.size() == 3 && !IsParkingTimeValid(ConvertTimeToMinutes(oneLineInput[1]),
            ConvertTimeToMinutes(oneLineInput[2]))) {
            cerr << "ERROR " << lineCounter << endl;
        }
        else {//Valid input.
            nextCurrentTime = ConvertTimeToMinutes(oneLineInput[1]);
            if (nextCurrentTime < currentTime) {//Move to the next day.
                carsParkedUntilToday = carsParkedUntilTomorrow;
                carsParkedUntilTomorrow.clear();
            }
            currentTime = nextCurrentTime;
            ParkedCarProcessing(carsParkedUntilToday, carsParkedUntilTomorrow,
                oneLineInput, lineCounter);
        }
    }
}

int main() {
    MainLogicLoop();
    return 0;
}
