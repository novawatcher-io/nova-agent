//
// Created by zhanglei on 2025/2/24.
//
#include "app/include/source/kubernetes/source.h"

namespace App::Source::Kubernetes {
void Source::init() {
    watcher->init();
}

void Source::start() {
    watcher->start();
};

void Source::stop() {
    watcher->stop();
};

void Source::finish() {

};

std::string Source::name(){
    return "kubernetes";
}

Source::~Source() {

}

}