/*
 *  Copyright (c) 2020 AnimatedLEDStrip
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <getopt.h>
#include <utility>
#include <vector>
#include <animatedledstrip/AnimationSender.hpp>

using namespace std;

struct Operation {
    int opId;
    string name;
    string description;
};

#define OP_ANIMATIONS 0
#define OP_END 1
#define OP_HELP 2
#define OP_INFO 3
#define OP_RUNNING 4
#define OP_START 5

int selectedOp = -1;
static struct Operation operations[6];

typedef option getopt_option;

struct Option {
    int optId;
    string identifier;

    getopt_option get_getopt_option() const {
        return getopt_option{identifier.c_str(), required_argument, nullptr, 0};
    }
};

#define OPT_SERVER 0
#define OPT_PORT 1
#define OPT_FORMAT 2
#define OPT_ANIMATION 3
#define OPT_COLOR 4
#define OPT_CENTER 5
#define OPT_CONTINUOUS 6
#define OPT_DELAY 7
#define OPT_DELAY_MOD 8
#define OPT_DIRECTION 9
#define OPT_DISTANCE 10
#define OPT_ID 11
#define OPT_SECTION 12
#define OPT_SPACING 13

#define NUM_OPTS 14

static struct Option options[NUM_OPTS];

string format;

AnimationSender * sender;
string serverIp;
int serverPort = -1;

string boolToString(bool b) {
    if (b) return "true";
    else return "false";
}

bool strlencmp(const pair<string, string> & a, const pair<string, string> & b) {
    return a.first.length() > b.first.length();
}

string replaceEscapedChars() {
    string retString;
    retString.assign(format);

    size_t found;

    while ((found = retString.find("\\n")) != string::npos) {
        retString.replace(found, 2, "\n");
    }

    while ((found = retString.find("\\t")) != string::npos) {
        retString.replace(found, 2, "\t");
    }

    return retString;
}

string populateFormatString(vector<pair<string, string>> replacements) {
    string retString;
    retString.assign(replaceEscapedChars());

    sort(replacements.begin(), replacements.end(), strlencmp);

    size_t found;

    for (const auto & replacement : replacements) {
        while ((found = retString.find(replacement.first)) != string::npos) {
            retString.replace(found, replacement.first.length(), replacement.second);
        }
    }

    return retString;
}


void printAnimationInfo(AnimationInfo anim) {

    vector<pair<string, string>> replacements = {
            {"%name",            anim.name},
            {"%n",               anim.name},
            {"%abbr",            anim.abbr},
            {"%a",               anim.abbr},
            {"%description",     anim.description},
            {"%d",               anim.description},
            {"%signatureFile",   anim.signatureFile},
            {"%f",               anim.signatureFile},
            {"%repetitive",      boolToString(anim.repetitive)},
            {"%r",               boolToString(anim.repetitive)},
            {"%R",               to_string(anim.repetitive)},
            {"%minimumColors",   to_string(anim.minimumColors)},
            {"%m",               to_string(anim.minimumColors)},
            {"%unlimitedColors", boolToString(anim.unlimitedColors)},
            {"%u",               boolToString(anim.unlimitedColors)},
            {"%U",               to_string(anim.unlimitedColors)},
            {"%center",          AnimationInfo::paramUsageToString(anim.center)},
            {"%ce",              AnimationInfo::paramUsageToString(anim.center)},
            {"%delay",           AnimationInfo::paramUsageToString(anim.delay)},
            {"%de",              AnimationInfo::paramUsageToString(anim.delay)},
            {"%direction",       AnimationInfo::paramUsageToString(anim.direction)},
            {"%dr",              AnimationInfo::paramUsageToString(anim.direction)},
            {"%distance",        AnimationInfo::paramUsageToString(anim.distance)},
            {"%ds",              AnimationInfo::paramUsageToString(anim.distance)},
            {"%spacing",         AnimationInfo::paramUsageToString(anim.spacing)},
            {"%sp",              AnimationInfo::paramUsageToString(anim.spacing)},
            {"%delayDefault",    to_string(anim.delayDefault)},
            {"%DE",              to_string(anim.delayDefault)},
            {"%distanceDefault", to_string(anim.distanceDefault)},
            {"%DS",              to_string(anim.distanceDefault)},
            {"%spacingDefault",  to_string(anim.spacingDefault)},
            {"%SP",              to_string(anim.spacingDefault)},
    };

    cout << populateFormatString(replacements) << endl;
}

void printAnimationData(AnimationData anim) {
    vector<pair<string, string>> replacements = {
            {"%animation",  anim.animation},
            {"%a",          anim.animation},
            {"%colors",     anim.colorsString()},
            {"%c",          anim.colorsString()},
            {"%center",     to_string(anim.center)},
            {"%ce",         to_string(anim.center)},
            {"%continuous", AnimationData::continuousToString(anim.continuous)},
            {"%co",         AnimationData::continuousToString(anim.continuous)},
            {"%delay",      to_string(anim.delay)},
            {"%de",         to_string(anim.delay)},
            {"%delayMod",   to_string(anim.delay_mod)},
            {"%dm",         to_string(anim.delay_mod)},
            {"%direction",  AnimationData::directionToString(anim.direction)},
            {"%dr",         AnimationData::directionToString(anim.direction)},
            {"%distance",   to_string(anim.distance)},
            {"%ds",         to_string(anim.distance)},
            {"%id",         anim.id},
            {"%i",          anim.id},
            {"%section",    anim.section},
            {"%se",         anim.section},
            {"%spacing",    to_string(anim.spacing)},
            {"%sp",         to_string(anim.spacing)},
    };

    cout << populateFormatString(replacements) << endl;
}

void printStripInfo(StripInfo info) {
    vector<pair<string, string>> replacements = {
            {"%numLEDs",           to_string(info.numLEDs)},
            {"%n",                 to_string(info.numLEDs)},
            {"%pin",               to_string(info.pin)},
            {"%p",                 to_string(info.pin)},
            {"%imageDebugging",    boolToString(info.imageDebugging)},
            {"%i",                 boolToString(info.imageDebugging)},
            {"%fileName",          info.fileName},
            {"%f",                 info.fileName},
            {"%rendersBeforeSave", to_string(info.rendersBeforeSave)},
            {"%r",                 to_string(info.rendersBeforeSave)},
            {"%threadCount",       to_string(info.threadCount)},
            {"%t",                 to_string(info.threadCount)},
    };

    cout << populateFormatString(replacements) << endl;
}

void setOutputRed() {
    cerr << "\033[31m";
}

void setOutputDefault() {
    cerr << "\33[0m";
}

void invalidFlag(int flag) {
    setOutputRed();
    cerr << "Flag ";
    setOutputDefault();
    cerr << options[flag].identifier;
    setOutputRed();
    cerr << " not allowed for operation ";
    setOutputDefault();
    cerr << operations[selectedOp].name << endl;
    exit(EXIT_FAILURE);
}

ColorContainer handleColorInput(const char * b) {
    string buff;
    buff.assign(b);

    auto cc = ColorContainer();
    int count = 0;
    int start = 0;

    while (true) {
        if (buff[start + count] == 0) {
            cc.addColor(strtol(buff.substr(start, count).c_str(), nullptr, 0));
            break;
        } else if (buff[start + count] == ',') {
            cc.addColor(strtol(buff.substr(start, count).c_str(), nullptr, 0));
            start += count + 1;
            count = 0;
        } else count++;
    }
    return cc;
}

Continuous handleContinuousInput(const char * b) {
    string buff;
    buff.assign(b);

    vector<pair<string, Continuous>> possibleArgs = {
            {"CONTINUOUS",    CONTINUOUS},
            {"TRUE",          CONTINUOUS},
            {"NONCONTINUOUS", NONCONTINUOUS},
            {"FALSE",         NONCONTINUOUS},
            {"DEFAULT",       DEFAULT},
            {"NULL",          DEFAULT},
    };

    // Change to uppercase
    for (char & c : buff) if (c >= 'a' && c <= 'z') c -= 32;

    vector<pair<string, Continuous>> matchingArgs;

    for (const auto & arg : possibleArgs) {
        for (int c = arg.first.length(); c > 0; c--) {
            if (strcmp(buff.c_str(), arg.first.substr(0, c).c_str()) == 0) {
                matchingArgs.push_back(arg);
                break;
            }
        }
    }

    if (matchingArgs.empty()) {
        setOutputRed();
        cerr << "Value ";
        setOutputDefault();
        cerr << b;
        setOutputRed();
        cerr << " does not match any of the following: ";
        setOutputDefault();
        for (const auto & arg : possibleArgs) cerr << arg.first << " ";
        cerr << endl;
        exit(EXIT_FAILURE);
    } else if (matchingArgs.size() > 1) {
        setOutputRed();
        cerr << "Value ";
        setOutputDefault();
        cerr << b;
        setOutputRed();
        cerr << " matches multiple values: ";
        setOutputDefault();
        for (const auto & arg : matchingArgs) cerr << arg.first << " ";
        cerr << endl;
        exit(EXIT_FAILURE);
    } else return matchingArgs[0].second;
}

Direction handleDirectionInput(const char * b) {
    string buff;
    buff.assign(b);

    vector<pair<string, Direction>> possibleArgs = {
            {"FORWARD",  FORWARD},
            {"BACKWARD", BACKWARD},
    };

    // Change to uppercase
    for (char & c : buff) if (c >= 'a' && c <= 'z') c -= 32;

    vector<pair<string, Direction>> matchingArgs;

    for (const auto & arg : possibleArgs) {
        for (int c = arg.first.length(); c > 0; c--) {
            if (strcmp(buff.c_str(), arg.first.substr(0, c).c_str()) == 0) {
                matchingArgs.push_back(arg);
                break;
            }
        }
    }

    if (matchingArgs.empty()) {
        setOutputRed();
        cerr << "Value ";
        setOutputDefault();
        cerr << b;
        setOutputRed();
        cerr << " does not match any of the following: ";
        setOutputDefault();
        for (const auto & arg : possibleArgs) cerr << arg.first << " ";
        cerr << endl;
        exit(EXIT_FAILURE);
    } else if (matchingArgs.size() > 1) {
        setOutputRed();
        cerr << "Value ";
        setOutputDefault();
        cerr << b;
        setOutputRed();
        cerr << " matches multiple values: ";
        setOutputDefault();
        for (const auto & arg : matchingArgs) cerr << arg.first << " ";
        cerr << endl;
        exit(EXIT_FAILURE);
    } else return matchingArgs[0].second;
}

void printHelp() {
    cout << "ledclient - Communicate with an AnimatedLEDStrip server" << endl;
    cout << "View the man page with 'man ledclient' for detailed help" << endl;
}


int main(int argc, char ** argv) {

    if (argc == 1) {
        setOutputRed();
        cerr << "Need to specify an operation";
        setOutputDefault();
        cerr << endl;
        exit(EXIT_FAILURE);
    }

    operations[OP_ANIMATIONS] = {OP_ANIMATIONS, "animations"};
    operations[OP_END] = {OP_END, "end"};
    operations[OP_HELP] = {OP_HELP, "help"};
    operations[OP_INFO] = {OP_INFO, "info"};
    operations[OP_RUNNING] = {OP_RUNNING, "running"};
    operations[OP_START] = {OP_START, "start"};

    options[OPT_SERVER] = {OPT_SERVER, "server"};
    options[OPT_PORT] = {OPT_PORT, "port"};
    options[OPT_FORMAT] = {OPT_FORMAT, "format"};
    options[OPT_ANIMATION] = {OPT_ANIMATION, "animation"};
    options[OPT_COLOR] = {OPT_COLOR, "color"};
    options[OPT_CENTER] = {OPT_CENTER, "center"};
    options[OPT_CONTINUOUS] = {OPT_CONTINUOUS, "continuous"};
    options[OPT_DELAY] = {OPT_DELAY, "delay"};
    options[OPT_DELAY_MOD] = {OPT_DELAY_MOD, "delayMod"};
    options[OPT_DIRECTION] = {OPT_DIRECTION, "direction"};
    options[OPT_DISTANCE] = {OPT_DISTANCE, "distance"};
    options[OPT_ID] = {OPT_ID, "id"};
    options[OPT_SECTION] = {OPT_SECTION, "section"};
    options[OPT_SPACING] = {OPT_SPACING, "spacing"};

    for (const auto & op : operations) {
        for (int c = op.name.length(); c > 0; c--) {
            if (strcmp(argv[1], op.name.substr(0, c).c_str()) == 0) {
                if (selectedOp == -1) selectedOp = op.opId;
                else {
                    setOutputRed();
                    cerr << "Multiple operations match ";
                    setOutputDefault();
                    cerr << argv[1] << endl;
                    exit(EXIT_FAILURE);
                }
                break;
            }
        }
    }

    if (selectedOp == -1) {
        setOutputRed();
        cerr << "Invalid operation: ";
        setOutputDefault();
        cerr << argv[1] << endl;
        exit(EXIT_FAILURE);
    }

    if (selectedOp == OP_HELP) {
        printHelp();
        exit(EXIT_SUCCESS);
    }

    auto newAnim = new AnimationData();
    auto newEnd = new EndAnimation();
    auto newCC = ColorContainer();
    int optIndex = -1;

    auto * opts = new getopt_option[NUM_OPTS + 1];
    for (const auto & opt : options) {
        opts[opt.optId] = opt.get_getopt_option();
    }
    opts[NUM_OPTS] = {nullptr, no_argument, nullptr, 0};

    int shortOpt;

    while ((shortOpt = getopt_long(argc, argv, "s:p:", opts, &optIndex)) != -1) {
        switch (shortOpt) {
            case 's':
                optIndex = OPT_SERVER;
                break;
            case 'p':
                optIndex = OPT_PORT;
                break;
            default:
                break;
        }

        switch (optIndex) {
            case OPT_SERVER:
                if (optarg) serverIp = string(optarg);
                break;
            case OPT_PORT:
                if (optarg) serverPort = (int) strtol(optarg, nullptr, 0);
                break;
            case OPT_FORMAT:
                switch (selectedOp) {
                    case OP_ANIMATIONS:
                    case OP_RUNNING:
                    case OP_INFO:
                        format = string(optarg);
                        break;
                    default:
                        invalidFlag(OPT_FORMAT);
                }
                break;
            case OPT_ANIMATION:
                if (selectedOp != OP_START) invalidFlag(OPT_ANIMATION);
                else newAnim->setAnimation(optarg);
                break;
            case OPT_COLOR:
                if (selectedOp != OP_START) invalidFlag(OPT_COLOR);
                else {
                    newCC = handleColorInput(optarg);
                    newAnim->addColor(newCC);
                }
                break;
            case OPT_CENTER:
                if (selectedOp != OP_START) invalidFlag(OPT_CENTER);
                else newAnim->setCenter((int) strtol(optarg, nullptr, 0));
                break;
            case OPT_CONTINUOUS:
                if (selectedOp != OP_START) invalidFlag(OPT_DIRECTION);
                else newAnim->setContinuous(handleContinuousInput(optarg));
                break;
            case OPT_DELAY:
                if (selectedOp != OP_START) invalidFlag(OPT_DELAY);
                else newAnim->setDelay(strtol(optarg, nullptr, 0));
                break;
            case OPT_DELAY_MOD:
                if (selectedOp != OP_START) invalidFlag(OPT_DELAY_MOD);
                else newAnim->setDelayMod(strtod(optarg, nullptr));
                break;
            case OPT_DIRECTION:
                if (selectedOp != OP_START) invalidFlag(OPT_DIRECTION);
                else newAnim->setDirection(handleDirectionInput(optarg));
                break;
            case OPT_DISTANCE:
                if (selectedOp != OP_START) invalidFlag(OPT_DISTANCE);
                else newAnim->setDistance((int) strtol(optarg, nullptr, 0));
                break;
            case OPT_ID:
                switch (selectedOp) {
                    case OP_START:
                        newAnim->setId(optarg);
                        break;
                    case OP_END:
                        newEnd->setId(optarg);
                        break;
                    default:
                        invalidFlag(OPT_ID);
                }
                break;
            case OPT_SECTION:
                if (selectedOp != OP_START) invalidFlag(OPT_SECTION);
                else newAnim->setSection(optarg);
                break;
            case OPT_SPACING:
                if (selectedOp != OP_START) invalidFlag(OPT_SPACING);
                else newAnim->setSpacing((int) strtol(optarg, nullptr, 0));
                break;
            default:
                setOutputRed();
                cerr << "Could not find operation for option with index ";
                setOutputDefault();
                cerr << optIndex << endl;
                exit(EXIT_FAILURE);
        }
    }

    if (serverIp.empty()) {
        setOutputRed();
        cerr << "Server IP must be set";
        setOutputDefault();
        cerr << endl;
        exit(EXIT_FAILURE);
    } else if (serverPort == -1) {
        setOutputRed();
        cerr << "Server port must be set";
        setOutputDefault();
        cerr << endl;
        exit(EXIT_FAILURE);
    }

    sender = new AnimationSender(serverIp, serverPort);

    switch (selectedOp) {
        case OP_ANIMATIONS:
            if (format.empty()) format = "%n";
            sender->setOnNewAnimationInfoCallback(printAnimationInfo);
            break;
        case OP_RUNNING:
            if (format.empty()) format = "%i\t%a";
            sender->setOnNewAnimationDataCallback(printAnimationData);
            break;
        default:
            break;
    }

    sender->start();

    switch (selectedOp) {
        case OP_START:
            sender->send(*newAnim);
            break;
        case OP_END:
            if (newEnd->id.empty()) {
                setOutputRed();
                cerr << "Animation ID must be set";
                setOutputDefault();
                cerr << endl;
                exit(EXIT_FAILURE);
            }
            sender->send(*newEnd);
            break;
        default:
            break;
    }

    sleep(1);

    sender->end();

    delete[] opts;
    delete newAnim;
    delete newEnd;
    delete sender;

    return 0;
}
