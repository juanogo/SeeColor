for i=1:2000
    j=ceil(rand(1)*399);k=ceil(rand(1)*399);
    temp=TEMP_U{j};
    TEMP_U{j}=TEMP_U{k};
    TEMP_U{k}=temp;
end

P=[];
for i=1:300
     pos{i}=imresize(im2double(rgb2gray(TEMP_U{i})), [16,13]);
     im=pos{i};
     P=[P im(:)];
end

P=P/max(max(P));

labelAll=zeros(5,1500);
labelAll(5,1201:1500)=labelAll(5,1201:1500)+1;
imagesc(labelAll)

for i=1:6
    pr=minmax(P);
    if i<6
        net=newff(pr,[100 2],{'logsig','logsig'},'traingdm');
    else
        net=newff(pr,[100 5],{'logsig','logsig'},'traingdm');
    end
    net.trainParam.epochs=7000;
    net.trainParam.goal=0.01;
    net.trainParam.lr=0.1;
    net.trainParam.mc=0.9;
    net.trainParam.show=100;
    [nets_shallow{i}.net,nets_shallow{i}.tr]=train(net,P,LABELS{i});
    save('NETS_SH','nets_shallow');
end
for i=1:6
  if i<6
    [nets_deep{i}.net, nets_deep{i}.tr]=deep_lear2(P,LABELS{i},[200 170 150 120 100 70 50 10 5 2]);
  else
    [nets_deep{i}.net, nets_deep{i}.tr]=deep_lear2(P,LABELS{i},[200 170 150 120 100 70 50 10 5]);
  end
  save('NETS_DE','nets_deep');
end
