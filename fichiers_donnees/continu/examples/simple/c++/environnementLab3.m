
clear all
close all
clc
setenv('TTKERNEL', 'X:\Bureau\tp3\fichiers_donnees\Fichiers necessaires\truetime-2.0-beta7\kernel')
addpath([getenv('TTKERNEL')])
addpath([getenv('TTKERNEL') '/matlab/help'])
addpath([getenv('TTKERNEL') '/matlab'])
truetime
mex -setup


% make_truetime    %pour la compilation de la librairie true time
% ttmex example_init.cpp       %pour la compilation du code du fichier C++