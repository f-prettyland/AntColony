dat = read.csv("/path/to/resultsNodes.csv")
plot(dat$nodes, dat$msec, xlab='Nodes', ylab='Time (ms)', col="red")
title(main = "Nodes vs Time for 20 Ants and 200 Iterations")

#finding how many find best
a <- dat[dat$SC.bs.==dat$bestSolnThroughout,]

dat = read.csv("/path/to/compiledParaVNorm.csv")
par <- dat[dat$parallelP==1&dat$nodes<40,]
seq <- dat[dat$parallelP==0&dat$nodes<40,]

plot(seq$nodes, seq$msec, xlab='Nodes', ylab='Time (ms)', col="blue")
points(par$nodes, par$msec, col="red")
title(main = "Nodes vs Time for 30 Ants and 300 Iterations (Nodes < 40)")
legend( x="bottomright", 
     legend=c("Parallel","Sequential"),
     col=c("red","blue", "green", "orange"), lwd=1)

par <- dat[dat$parallelP==1,]
seq <- dat[dat$parallelP==0,]

plot(seq$nodes, seq$msec, xlab='Nodes', ylab='Time (ms)', col="blue")
points(par$nodes, par$msec, col="red")
title(main = "Nodes vs Time for 30 Ants and 300 Iterations")
legend( x="bottomright", 
     legend=c("Parallel","Sequential"),
     col=c("red","blue", "green", "orange"), lwd=1)


# a <- dat[dat$Model=="" & dat$Heuristic==" static" & dat$Opt==0,]