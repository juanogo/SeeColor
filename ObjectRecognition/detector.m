function detector(name,opt)

clc, 
load (name)
addpath(genpath('.'));
i=size(tld.source.idx,2);
I = tld.source.idx(i);
fern(7,tld.SIPj,tld.Wj,tld.NPj,tld.NNj,tld.features,tld.scales, tld.grid );

obj = videoinput('winvideo', 1,'YUY2_320x240');
set(obj, 'SelectedSourceName', 'input1')
set(obj,'ReturnedColorSpace','grayscale');
preview(obj);
set(1,'visible','off');

figure(2); set(2,'KeyPressFcn', @handleKey); % open figure for display of results
finish = 0; function handleKey(~,~), finish = 1; end % by pressing any key, the process will exit
finish=0;
while (1)
    tld.img{I} = img_alloc(getsnapshot(obj));
    [dBB dConf tld] = tldDetection(tld,I);
    imshow(tld.img{I}.input)
    hold on
    if tld.plot.dt && ~isempty(tld.dt{i})
        cp = bb_center(tld.dt{i}.bb);
        if ~isempty(cp)
            plot(cp(1,:),cp(2,:),'.','color',0.25*[1 1 1]);
        end
        idx = tld.dt{i}.conf1 > tld.model.thr_nn;
        cp = bb_center(tld.dt{i}.bb(:,idx));
        if ~isempty(cp)
            plot(cp(1,:),cp(2,:),'.','color','red');
             if opt==1
             wavplay(tts(tld.name,[],-4,44100),44100, 'async')
             elseif opt==2
             wavplay(rand(1,1000),84100, 'async')
             end
        end
    end
    hold off
    if finish % finish if any key was pressed
        delete(obj), close all
        return;
    end
    drawnow
end

delete(obj)

end