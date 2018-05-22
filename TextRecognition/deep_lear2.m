function [net, tr]= deep_lear2(P,T,lay)
%Trains a network with layer-wise greedly unsupervised pretraining.



gP=P; gT=T; T=P; clc,
for i=1:size(lay,2)-1
    pr=minmax(P);
    net=newff(pr,[lay(i) size(P,1)],{'logsig','logsig'},'traingdm');
    
    net.trainParam.epochs=7000;
    net.trainParam.goal=0.01;
    net.trainParam.lr=0.1;
    net.trainParam.mc=0.9;
    net.trainParam.show=100;
    
    clc,disp(['pre-training layer ', num2str(i), ' out of ',num2str(size(lay,2)-1),  '...'])
    if (lay(i) == size(P,1))
        net.LW{2,1}=net.IW{1}';
    end
   
    [net,~]=train(net,P,T);
    P=logsig(repmat(net.b{1},1,size(P,2))+(net.IW{1}*P));
    T=P;
    
    W{i}= net.IW{1};B{i}=net.b{1};
end



P=gP; T=gT;
disp('unsupervised pre-training finished!')
pr=minmax(P);
for i=1:size(lay,2)
  %LAYERS(i)= inp;
  FUN {i}= 'logsig';
end
%LAYERS(end)=size(T,1);
net=newff(pr,lay,FUN,'traingdm');
net.trainParam.epochs=7000;
net.trainParam.goal=0.001;
net.trainParam.lr=0.1;
net.trainParam.mc=0.9;
net.trainParam.show=100;
net.trainParam.min_grad=1e-16;
net.IW{1}=W{1};
net.b{1}=B{1};
id=2;
for i=2:size(lay,2)-1
    for j=1:size(lay,2)-2
        if j==i-1;
        net.LW{i,j}=W{id};
        net.b{id}=B{id};
        id=id+1;
        end
    end
end

disp('undergoing supervised training of the entire network...')
[net,tr]=train(net,P,T);
disp('Deep Learning finished!')
